
#include <draw.h>
#include <unistd.h>
#include <math.h>

typedef struct {
	double x;
	double y;
} vector;

typedef struct {
	int radius;
	vector position;
	vector velocity;
	uint32_t color;
} ball;


void wall_collision(ball *b){ // handles the collision and moves the guy back if needed
	if ((b->position.x - b->radius) < 0    ) {b->velocity.x *= -1; b->position.x = 0 + b->radius;}
	if ((b->position.x + b->radius) > 1920 ) {b->velocity.x *= -1; b->position.x = 1920-b->radius;}
	if ((b->position.y - b->radius) < 0    ) {b->velocity.y *= -1; b->position.y = 0 + b->radius;}
	if ((b->position.y + b->radius) > 1080 ) {b->velocity.y *= -1; b->position.y = 1080-b->radius;}
}

double dot (vector v1, vector v2){// specifically used for velocities
	return v1.x * v2.x + v1.y * v2.y;
}

double mag (vector v){ // magnitude of a vector
	return sqrt(pow(v.x, 2) + pow(v.y, 2));	
}

vector difference (vector a, vector b){
	vector diff;
	diff.x = a.x - b.x;
	diff.y = a.y - b.y;
	return diff;
}

vector projection(vector a, vector b){ // projection of a onto b
	vector proj;
	double scalar = dot(a, b)/(mag(b)*mag(b));
	proj.x = scalar * b.x;
	proj.y = scalar * b.y;
	return proj;
}

int touching (ball b1, ball b2){ 
	return ( (b1.radius + b2.radius) > mag(difference(b1.position, b2.position)));
}

void clear_ball(ball *b){
		fill_circle(b->position.x, b->position.y, b->radius, BLACK);
}

void draw_ball(ball *b){
		fill_circle(b->position.x, b->position.y, b->radius, b->color);
}

void rewind_overlap(ball *b1, ball *b2){
	while(touching(*b1, *b2)){
		b1->position.x -= 0.1*b1->velocity.x;
		b1->position.y -= 0.1*b1->velocity.y;
		b2->position.x -= 0.1*b2->velocity.x;
		b2->position.y -= 0.1*b2->velocity.y;
	}
}

void collision(ball *b1, ball *b2){ // sends balls back after a collision
	
	vector normal = difference(b1->position, b2->position);
	
	vector tangent;
	tangent.x = -normal.y;
	tangent.y = normal.x;
	
	// should only turn them around if they are going in opposite directions
	// what about it just sums their normal components?

	vector b1_normal = projection(b1->velocity, normal);
	vector b1_tangent = projection(b1->velocity, tangent);
	vector b2_normal = projection(b2->velocity, normal);
	vector b2_tangent = projection(b2->velocity, tangent);
	
	vector b1_temp;
	b1_temp.x = b1_normal.x;
	b1_temp.y = b1_normal.y;

	b1_normal.x = b2_normal.x;
	b1_normal.y = b2_normal.y;
	b1->velocity.x = b1_normal.x + b1_tangent.x;
	b1->velocity.y = b1_normal.y + b1_tangent.y;
	
	b2_normal.x = b1_temp.x;
	b2_normal.y = b1_temp.y;
	b2->velocity.x = b2_normal.x + b2_tangent.x;
	b2->velocity.y = b2_normal.y + b2_tangent.y;
}

void update_position(ball *b){
		b->position.x += b->velocity.x;
		b->position.y += b->velocity.y;
}

int main(){
	buffer_init();
	//clear_buffer();

	ball b;
	b.radius = 30;
	b.position.x = 1000;
	b.position.y = 1000;
	b.velocity.x = 0.3;
	b.velocity.y = 0.4;
	b.color = 0xffaa33;

	ball c;
	c.radius = 30;
	c.position.x = 1040;
	c.position.y = 300;
	c.velocity.x = 0.3;
	c.velocity.y = -0.2;
	c.color = 0xff2233;

	ball d;
	d.radius = 30;
	d.position.x = 340;
	d.position.y = 600;
	d.velocity.x = 0.5;
	d.velocity.y = 0.2;
	d.color = 0x3366ff;

	ball e;
	e.radius = 30;
	e.position.x = 340;
	e.position.y = 1000;
	e.velocity.x = 0.2;
	e.velocity.y = -0.3;
	e.color = 0x33ff77;

	ball balls[] = {b, c, d, e};
	int ball_count = 4;

	int i = 1000000;
	while (i--){
		for (int j = 0; j < ball_count; j++){
			clear_ball(&balls[j]);
			for (int other = j+1; other < ball_count ; other++){
				if (touching(balls[j], balls[other])){
					clear_ball(balls + other);
					rewind_overlap(&balls[j], &balls[other]);
					collision(&balls[j], &balls[other]);
				}
			}
			wall_collision(&balls[j]);
			update_position(&balls[j]);
			draw_ball(balls + j);
		}
		usleep(1000);
	}

	buffer_reset();
}
