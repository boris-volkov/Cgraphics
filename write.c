#include "draw.h"

int main() {
	init_font();
	set_raw_mode();
	buffer_init();

	clear_buffer();	
	fill_rect( 80, 80, 300, 600, 0x00ff5511);
	fill_circle( 100, 100, 100, 0x00ff5511);
	draw_char(100, 100, '8', 0, 30);



	sleep(4); //  the arch terminal is a bit more aggressive in redrawing the terminal
	free_font();
	reset_terminal_mode();
	buffer_reset();
}
