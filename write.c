#include "draw.h"

struct cursor {
	int x;
	int y;
};

void highlighted_char(struct cursor *c, size_t chr, int scale){
	fill_rect( c->x-scale, c->y + scale , scale*9, scale*16, 0x00ff5511);
	draw_char(c->x, c->y, chr, 0, scale);
	c->x += scale*9;
	if (c->x > 1980 - scale*9){
		c->x = 0;
		c->y += scale* 16;
	}
}

int main() {
	init_font();
	set_raw_mode();
	buffer_init();
	struct cursor c;
	c.x = 0;
	c.y = 0;
	
	clear_buffer();
	for (size_t i = 0; i < 1000; i++){
		highlighted_char(&c, i, 2);
	}

	sleep(4); //  the arch terminal is a bit more aggressive in redrawing the terminal
	free_font();
	reset_terminal_mode();
	buffer_reset();
}
