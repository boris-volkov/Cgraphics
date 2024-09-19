#include "life.h"
#include "draw.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

/**************************************
* 	GAME OF LIFE CORE RULES:
*	live cell with two or three neighbors survives
*	dead cell with three live neighbors becomes alive
* all other live cells die
* this is set up so that you can alter the rules mid-game
***************************************/

unsigned char rules[2][9]  =  // [living or dead?] [neighbor count]
{
	// 0 1 2 3 4 5 6 7 8 
		{0,0,0,1,0,0,0,0,0},
		{0,0,1,1,0,0,0,0,0}
};

unsigned char *this_gen;	
unsigned char *next_gen;
unsigned char *temp;
int grid_rows;
int grid_cols;
int cell_px;
size_t generation = 1; // starting at the first generation obviously 
uint32_t this_color = YELLOW;


struct grid {}; // TODO 
const double PI = 3.1415;

// create a color based on the current generation
// TODO have the color be set based on the age of the cell,
// so ones that have been fixed the longest dim/grey out like rocks
// but ones that are fresh are green with life
void gen_color(){
	uint8_t r = 40 * sin ( 2*PI*(generation+16)/ 256)   + 130;
	uint8_t g = 15 * sin ( 2*PI*(generation+64) /256)   + 125;
	uint8_t b = 30 * sin ( 2*PI*(generation+128)  /256)  + 180;
	this_color = color(r, g, b);
}


void draw_cell_square(int row, int col, uint32_t color){
	if (cell_px >=5){ // draw it within the border if the squares are large enough to render a border
		fill_rect(col*cell_px+1, row*cell_px+1, cell_px-1, cell_px-1, color);
	} else {
		fill_rect(col*cell_px, row*cell_px, cell_px, cell_px, color);	// no borders at this size
	}
}


int menu_status;

Cursor menu_cursor;
Cursor cursor;
// takes cell size in pixels and allocates the right amount of memory
// calls to draw the grid to the screen too
void initialize_grid(int cell_size_px){
	menu_status = 0; // flag for whether we are in the menu state or not
	srand(time(NULL));
	gen_color();
	cell_px = cell_size_px;
	int pixels = vinfo.xres * vinfo.yres;
	grid_rows = vinfo.yres / cell_size_px; // TODO make grid an object with handle that includes this stuff 
	grid_cols = vinfo.xres / cell_size_px;
	this_gen = (unsigned char *) malloc(sizeof(unsigned char) * pixels * 2); // allocating more memory than we need... 
	// allowing for every pixel of the screen to be 
	// Practical C says consider making every large array in the heap.
	// would have to just allocate the fixed 1080x1920 and and let it be 
	next_gen = this_gen + pixels;
}


void randomize_grid(){
	for (int row = 0; row < grid_rows; row++){
		for (int col = 0; col < grid_cols; col++){
			if(rand() % 7 == 0)
				next_gen[row* grid_cols + col] = 1;
		}
	}
}


void toggle(int row, int col){
	this_gen[row * grid_cols + col] ^= 1;
	cell(row,col) ? 
				draw_cell_square(row, col, this_color):
				draw_cell_square(row, col, BLACK);
}


unsigned char cell(int row, int col){
	return this_gen[row * grid_cols + col];
}


// special mod that SOME languages need because their
// % operator does not really behave like mod for negative numbers
int mod(int a, int b){ 
	return (a % b + b) % b;
}

inline int neighbor_count(int row, int col){
	// calculates neighbors by torus orientation
	int total = 0;
	for (int i = -1; i < 2; i++){
		for (int j = -1; j < 2; j++){
			total += cell(mod(row+i,grid_rows), mod(col+j,grid_cols));
		}
	}
	total -= cell(row, col); // so that alive cells don't count themselves among neighbors
	return total;
}


// this function is really hard to read
void compute_next_gen(){
	generation++;
	gen_color();
	for (int row = 0; row < grid_rows; row++){
		for (int col = 0; col < grid_cols; col++){
			next_gen[row* grid_cols + col] = rules[ cell(row, col) ][ neighbor_count(row, col) ];
		/*	if (next_gen[row*grid_cols + col] == 1 && !cell(row, col)){ //  only color it if it is not already 
				draw_cell_square(row, col, this_color);
			} else if (next_gen[row*grid_cols + col] == 0 && cell(row, col)){	// only black it out if its not already dead
				draw_cell_square(row, col, BLACK);
			}*/
		}
	}
}

void swap_generations(){
	temp = this_gen;
	this_gen = next_gen;
	next_gen = temp;
}

void all_dead(){
	for (int row = 0; row < grid_rows; row++){
		for (int col = 0; col < grid_cols; col++){
			next_gen[row* grid_cols + col] = 0;
		}
	}
}

void draw_gen(unsigned char* gen){
	for (int row = 0; row < grid_rows; row++){
		for (int col = 0; col < grid_cols; col++){
			if (gen[row*grid_cols + col] == 1){
				draw_cell_square(row, col, this_color);
			} else {	
				draw_cell_square(row, col, BLACK);
			}
		}
	}
}

void move_cursor(int x, int y, Cursor *cursor){
	// clear old position of the cursor
	if ( ( x || y ) ){ // if we moved, clear old position 
		if (cell_px <= 10){ 
		// no grid at this scale
			trace_rect(cursor->x*cell_px, cursor->y*cell_px, cell_px, cell_px, BLACK); 
		} else {
			trace_rect(cursor->x*cell_px, cursor->y*cell_px, cell_px, cell_px, BLUE);
		}
	}
	cursor->y += y;
	cursor->x += x;
	// draw the new cursor
	trace_rect(cursor->x*cell_px, cursor->y*cell_px, cell_px, cell_px, YELLOW);
}

void center_cursor(Cursor *cursor){
	int x_offset = grid_cols/2 - cursor->x;
	int y_offset = grid_rows/2 - cursor->y;
	move_cursor(x_offset, y_offset, cursor);
}

void toggle_rule(Cursor *cursor){
	rules[cursor->y][cursor->x] ^= 1;
}


// menu stuff is over here and is starting to feel like it should be part of a separate file

int menu_px; // this is the pixel size of the selection box in the menu
int menu_x;
int menu_y;

uint32_t MENU_BLUE = 0x335577;
void move_menu_cursor(int x, int y, Cursor *cursor){
	trace_rect(	menu_x + cursor->x*menu_px, 
							menu_y + cursor->y*menu_px, 
							menu_px, menu_px, MENU_BLUE);
	cursor->x += x;
	cursor->y += y;
	if (cursor->x < 0) cursor->x = 0; // clumsily put keep it in the correct range
	if (cursor->x > 8) cursor->x = 8;
	if (cursor->y < 0) cursor->y = 0;
	if (cursor->y > 1) cursor->y = 1;
	trace_rect(	menu_x + cursor->x * menu_px, 
							menu_y + cursor->y * menu_px, 
							menu_px, menu_px, YELLOW);
}

void draw_rules(){
	//assert(menu_status == 1);
	for (int i = 0; i < 2; i++){
		for (int j = 0; j < 9; j++){
			draw_char(menu_x + j*menu_px + 8, menu_y - 32, '0' + j, BLACK, 2);
			if (rules[i][j]){
				fill_rect(menu_x + j*menu_px + 4, 
									menu_y + i*menu_px + 4, 
									menu_px - 8, 
									menu_px - 8, 
									YELLOW);	
			} else {  
				fill_rect(menu_x + j*menu_px + 4, 
									menu_y + i*menu_px + 4, 
									menu_px - 8, 
									menu_px - 8, 
									BLACK);	
			}
		}
	}
}

// draws a menu over the screen so that user can change the rule mid-game
void menu(){
	menu_px = 30;
	menu_x = vinfo.xres/2 - 4*menu_px; // raw pixel offset
	menu_y = vinfo.yres/2 - menu_px;
	fill_rect(menu_x-30, 
						menu_y-40, 
						menu_px*9+60, 
						menu_px*2+60, 
						MENU_BLUE); 
	draw_rules();
	move_menu_cursor(0, 0, &menu_cursor);
}

////////////////////*****************************************************


void *input_handler(void *arg) {
	char c;
	center_cursor(&cursor);
	while (1) {
		c = getchar();

		if (menu_status) { // key bindings for controlling the menu 
			switch (c) {
				// continue within a switch would apply to the surrounding loop
				// if the arrow cases are going to fall through, use the  /* Fall Through */ instead of a break for clarity;
				case 's':
					move_menu_cursor(0,1, &menu_cursor);
					break;
				case 'w':
					move_menu_cursor(0,-1, &menu_cursor);
					break;
				case 'd':
					move_menu_cursor(1,0, &menu_cursor);
					break;
				case 'a':
					move_menu_cursor(-1,0, &menu_cursor);
					break;
				case '\n':
					toggle_rule(&menu_cursor);
					draw_rules();
					break;
				case 'm':
					// TODO this guy shouldn't be responsible for resetting the whole screen
					menu_status = 0;
					clear_buffer();
					grid(cell_px, cell_px);
					draw_gen(this_gen);
					break;
				default:
					break;
			}

		} else { // key bindings for normal game control

			switch (c){
				case 'Q':
					return NULL;
				case 's':
					move_cursor(0,1, &cursor);
					break;
				case 'w':
					move_cursor(0,-1, &cursor);
					break;
				case 'd':
					move_cursor(1,0, &cursor);
					break;
				case 'a':
					move_cursor(-1,0, &cursor);
					break;
				case '?':
					randomize_grid();
					draw_gen(next_gen);
					swap_generations();
					break;
				case '\n':
					toggle(cursor.y, cursor.x);
					break;
				case 'x':
					all_dead();
					draw_gen(next_gen);
					swap_generations();
					move_cursor(0,0, &cursor);
					break;
				case 'c':
					center_cursor(&cursor);
					break;
				case 'n':
					// this is to hide the cursor but should probably be separate function with a check for size
					trace_rect(cursor.x*cell_px, cursor.y*cell_px, cell_px, cell_px, BLUE);
					compute_next_gen(); 
					draw_gen(next_gen);
					swap_generations();
					break;
				case 'm':
					menu_status = 1;
					menu();
					break;
				case 'p':
					screenshot_to_ppm();
					break;
			}
		}
	}
	return NULL;
}
