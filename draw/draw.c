#include <stdlib.h>
#include <stdint.h>
#include "draw.h"
#include <sys/mman.h>

// definitions: these are what actually allocate the memory for 
// variables that are declared in the header
struct fb_var_screeninfo vinfo; // variable screen info struct
uint32_t * fbdata; //raw frame buffer data, one int32 pre pixel 0x00rrggbb
int fbfd; // frame buffer file descriptor
int tty_fd; // teletype file descriptor, for taking control of the terminal 
int fb_data_size;


#define return_defer(value) do { result = (value); goto defer; } while (0)
// interesting way from tsoding to handle error messages and jumping in a function 
//

void screenshot_to_ppm(){
	FILE *f = fopen("picture.ppm", "wb");
	if (f == NULL) goto defer;

	fprintf(f, "P6\n%d %d 255\n", vinfo.xres, vinfo.yres);
	for (size_t i = 0; i < vinfo.xres*vinfo.yres; ++i) {
		uint32_t pixel = fbdata[i];
		uint8_t bytes [3] = {
			(pixel>>(8*0))&0xFF,
			(pixel>>(8*1))&0xFF,
			(pixel>>(8*2))&0xFF
		};
		fwrite(bytes, sizeof(bytes), 1, f);
	}
defer:
	if (f) fclose(f);		
}

//TODO use the frame buffer info to generalize rgb offsets
// right now this only works for ARGB format
uint32_t color (uint8_t r, uint8_t g, uint8_t b){
	return (r<<16) + (g<<8) + (b);
}

inline void draw(int x, int y, uint32_t color){
			if (x < 0 || x >= vinfo.xres) return;
			if (y < 0 || y >= vinfo.yres) return; 
			fbdata[y*vinfo.xres + x] = color;
}

void fill_rect(int x, int y, int w, int h, uint32_t color){
	for (int row = y ; row < y+h; row++){
		for (int col = x ; col < x+w ; col++){
			draw(col, row, color);
		}
	}
}	

void draw_horizontal_line(int x1, int x2, int y, uint32_t pixel){
	for (int i = x1; i < x2; i++)
		draw(i, y, pixel);
}


void draw_vertical_line(int x, int y1, int y2, uint32_t pixel){
	for (int i = y1; i < y2; i++)
		draw(x, i, pixel);
}

//Draw a circle at (cx,cy)
void draw_circle(double cx, double cy, int radius, uint32_t color){
	inline void plot4points(double cx, double cy, double x, double y, uint32_t color){
		draw(cx + x, cy + y,color);
		draw(cx - x, cy + y,color);
		draw(cx + x, cy - y,color);
		draw(cx - x, cy - y,color);
	}

	inline void plot8points(double cx, double cy, double x, double y, uint32_t color){
		plot4points(cx, cy, x, y,color);
		plot4points(cx, cy, y, x,color);
	}

	int error = -radius;
	double x = radius;
	double y = 0;

	while (x >= y){
		plot8points(cx, cy, x, y, color);

		error += y;
		y++;
		error += y;

		if (error >= 0){
			error += -x;
			x--;
			error += -x;
		}
	}
}

//fill a circle at (cx,cy)
void fill_circle(double cx, double cy, int radius, uint32_t color){
	inline void plot4points(double cx, double cy, double x, double y, uint32_t pixel){
		draw_horizontal_line(cx - x, cx + x, cy + y,pixel);
		draw_horizontal_line(cx - x, cx + x, cy - y,pixel);
	}

	inline void plot8points(double cx, double cy, double x, double y, uint32_t color){
		plot4points(cx, cy, x, y,color);
		plot4points(cx, cy, y, x,color);
	}

	int error = -radius;
	double x = radius;
	double y = 0;

	while (x >= y){
		plot8points(cx, cy, x, y, color);

		error += y;
		y++;
		error += y;

		if (error >= 0){
			error += -x;
			x--;
			error += -x;
		}
	}
}

void trace_rect(int x, int y, int w, int h, uint32_t color){
	if (w <= 2 || h <= 2) return; // too small to trace a rectangle
	// but then would we want to just fill the rectangle?
	draw_vertical_line(x, y, y+h, color);
	draw_vertical_line(x+w, y, y+h, color);
	draw_horizontal_line(x, x+w, y, color);
	draw_horizontal_line(x, x+w, y+h, color);
}

void clear_buffer(){
	fill_rect(0, 0, vinfo.xres, vinfo.yres, 0);
}

void grid(int w, int h){
	if (w <= 10 || h <=10) return; // this is too specific
	for (int i = 0; i < vinfo.yres; i+=h){
		draw_horizontal_line(0, vinfo.xres, i, BLUE);
	}
	for (int i = 0; i < vinfo.xres; i+=w){
		draw_vertical_line(i, 0, vinfo.yres, BLUE);
	}
} 

void buffer_init(){
	char * ttypath = ttyname(STDIN_FILENO);
	tty_fd = open(ttypath, O_RDWR); 
	ioctl (tty_fd, KDSETMODE, KD_GRAPHICS); 
	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd < 0){
		perror("could not open frame buffer device");
		return;
	}

	ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo);

	// figure out how much space to map 
	int fb_width = vinfo.xres;
	int fb_height = vinfo.yres;
	int fb_bpp = vinfo.bits_per_pixel;
	int fb_bytes = fb_bpp/8;
	fb_data_size = fb_width * fb_height * fb_bytes;

	fbdata = mmap ( 0, fb_data_size, 
			PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0);
}

void buffer_reset(){
	munmap (fbdata, fb_data_size); // must unmap before closing!
  ioctl(tty_fd,KDSETMODE,KD_TEXT); // give control back to the text terminal 
	close (fbfd);
}

/*
* this is the section for font related stuff
*/

struct psf1_header {
	unsigned char magic[2]; // Magic bytes
	unsigned char mode;     // PSF mode
	unsigned char charsize; // Character size in bytes
};

struct psf1_header font;
unsigned char *font_data;

#define PSF1_MAGIC0 0x36 	//54
#define PSF1_MAGIC1 0x04	// 4

int scale = 20;
void draw_char(int x, int y, size_t c, uint32_t color, int scale) {
	// Assume 'font_data' contains the bitmap for character 'c'
	unsigned char *glyph = &font_data[c * font.charsize];

	for (int row = 0; row < font.charsize; row++) {  // through the rows of the individual glyph
		for (int col = 0; col < 8; col++) { // through columns of the glyph (as bits) 
			if (glyph[row] & (1 << (7 - col))) {  // if the bit at that "column" is lit up
						fill_rect(x + col*scale, y + row*scale, scale, scale, color);
			}
		}
	}
}

void init_font(){
	//TODO the font file is small enough that we can just store the font array in memory
	// can probably just hard-code it as a bunch of long long's or something. 
	FILE *font_file = fopen("Terminus16.psf", "rb");
	if (!font_file) {
		perror("Error opening font");
		exit(1);
	}
	// fread is being used here because it is a standard library function that is 
	// generalizable to other operating systems, whereas open() is a Unix system call
	// fread automatically buffers which can be useful for big files, but this font is not particularly big
	// fread is more portable. also operates on FILE* pointers which are more robust than int

	fread(&font, sizeof(struct psf1_header), 1, font_file); // pick off the header at the top of the font file
	// to see if the font is of the correct format. 
	if (font.magic[0] != PSF1_MAGIC0 || font.magic[1] != PSF1_MAGIC1) {
		fprintf(stderr, "Invalid font format\n");
		// wrong to exit here, what do we want to do if the font is not working?
		// wouldn't hurt to just hard-code the font data into the printing, no?
		exit(1);
	}
	
	size_t font_size = font.charsize * 256;
	font_data = malloc(font_size);
	// if we are calling here, the file pointer will have been pushed past the header by the previous read. 
	fread(font_data, font_size, 1, font_file); // font data is a char pointer to hold 256 characters
	fclose(font_file); // don't need the file any more since the font is in memory now.
}

void free_font(){
	free(font_data);
}

// This section sets the terminal to raw mode and back
// used to be in its own file, but since its so closely tied to setting up 
// the frame buffer I decided for now to leave it all here

struct termios original; // this struct holds on to the standard terminal settings
struct termios raw;

void set_raw_mode() {

	// Get current terminal attributes
	tcgetattr(STDIN_FILENO, &original);

	// Make a copy of the current state for modification 
	tcgetattr(STDIN_FILENO, &raw);

	// Turn off canonical mode and echo in the copy
	// ISIG to Turn off signal generation to not accidentally kill with 
	// Ctrl-C and skip all the cleanup
	raw.c_lflag &= ~(ICANON | ECHO); 
	
	// Set the attirutes in the actual settings
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void reset_terminal_mode() {
	// restore the original settings 
	tcsetattr(STDIN_FILENO, TCSANOW, &original);
}


