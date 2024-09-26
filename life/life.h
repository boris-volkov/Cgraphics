#ifndef LIFE_H
#define LIFE_H

#include <stdint.h>

//TODO obviously these should be dynamically set 
#define ROWS  (1920/20)
#define COLS  (1080/20)

extern unsigned char *this_gen;
extern unsigned char *next_gen;
extern unsigned char *temp;

extern unsigned char rules[2][9];  // [living or dead?] [neighbor count]

extern int cell_px;

// special mod that SOME languages need because their
// % operator does not really behave like mod for negative numbers
int mod(int a, int b); 

void initialize_grid(int);

void toggle(int row, int col);

unsigned char cell(int row, int col);

int neighbor_count(int row, int col);

void compute_next_gen();

void swap_generations();

void draw_cell_square(int row, int col, uint32_t color);
void draw_next_gen();

void *input_handler(void *);
typedef struct{
	int x; 
	int y;
} Cursor; // just holds the x, y of the cursors 

#endif // LIFE_H
