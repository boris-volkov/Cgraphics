#ifndef DRAW_H
#define DRAW_H

#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <termios.h>
#include <stdint.h>

//                  A R G B 
#define GREEN 	 	0x0011aa44
#define RED 	 	 	0x00ff3333
#define YELLOW   	0x00dd9911
#define BLUE 	 		0x00051122
#define BLACK 	 	0x0

// these are variables that we tell the compiler WILL EXIST,
// but these are not actually allocating any memory, they are only declarations
// putting the same line in the .c file is the definition that allocates memory. 


// raw frame buffer data... how much of this really needs to be externally available?
extern struct fb_var_screeninfo vinfo; // variable screen info struct


// Functions for getting access to the frame buffer and setting the keyboard input to raw mode 
void buffer_init(); // initializes fbdata and vinfo for use throughout the program
void buffer_reset(); // unmap the buffer and give control back to the terminal
void set_raw_mode(); // function to keep keypresses for echoing in the terminal
void reset_terminal_mode(); // return the terminal to its normal (canonical) mode
void init_font(); // set up the font for using draw_char();
void free_font(); // just closes the font file;


// basic functions for drawing to the frame buffer, points lines and rectangles
uint32_t color (uint8_t r, uint8_t g, uint8_t b);
void draw(int x, int y, uint32_t color); // draws a single pixel to the frame buffer
void fill_rect(int x, int y, int w, int h, uint32_t color);
void draw_horizontal_line(int x1, int x2, int y, uint32_t pixel);
void draw_vertical_line(int x, int y1, int y2, uint32_t pixel);
void trace_rect(int x, int y, int w, int h, uint32_t color);
void clear_buffer();
void grid(int w, int h); // width and height of the grid in pixels
void draw_char(int x, int y, size_t c, uint32_t color, int scale); //scale multiplies the base 16 height font

void draw_circle(double cx, double cy, int radius, uint32_t color);
void fill_circle(double cx, double cy, int radius, uint32_t color);

void screenshot_to_ppm(void);

#endif //draw_h
