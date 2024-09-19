#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "draw.h"
#include "life.h"


int main(int argc, char *argv[]) {

	// these three should probably be packaged together. 
	init_font();
	set_raw_mode();
	buffer_init();	

	int square_size = 20; // default square size
	if (argc > 1){ // let user select a square size
		square_size = atoi(argv[1]);
	}

	initialize_grid(square_size);
	clear_buffer();
	if (cell_px > 5) { // don't stroke the grid if the squares are too small
		grid(cell_px, cell_px); // Im using grid to refer to the line grid as well as the actual life matrix
	}


	// a speparate thread is started for input handling 
	pthread_t input_thread;
	if (pthread_create(	&input_thread, NULL, input_handler, NULL) != 0) {
		perror("failed to create thread\n");
		return 1;
	}	

	// pthread_join waits for the thread to finish,
	// then we run the cleanup functions.
	pthread_join(input_thread, NULL);
	free_font();
	reset_terminal_mode();
	buffer_reset();

	return 0;
}

