
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
} ball;


void wall_collision(ball *b){
	if ((b->position.x - b->radius) < 0    ) {b->velocity.x *= -1; b->position.x = 0 + b->radius;}
	if ((b->position.x + b->radius) > 1920 ) {b->velocity.x *= -1; b->position.x = 1920-b->radius;}
	if ((b->position.y - b->radius) < 0    ) {b->velocity.y *= -1; b->position.y = 0 + b->radius;}
	if ((b->position.y + b->radius) > 1080 ) {b->velocity.y *= -1; b->position.y = 1080-b->radius;}
}

double dot (vector v1, vector v2){// specifically used for velocities
	return v1.x * v2.x + v1.y * v2.y;
}

double mag (vector v){
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

void collision(ball *b1, ball *b2){
	vector normal = difference(b1->position, b2->position);
	
	vector tangent;
	tangent.x = -normal.y;
	tangent.y = normal.x;

	vector b1_normal = projection(b1->velocity, normal);
	vector b1_tangent = projection(b1->velocity, tangent);
	b1_normal.x *= -1;
	b1_normal.y *= -1;
	b1->velocity.x = b1_normal.x + b1_tangent.x;
	b1->velocity.y = b1_normal.y + b1_tangent.y;
	
	vector b2_normal = projection(b2->velocity, normal);
	vector b2_tangent = projection(b2->velocity, tangent);
	b2_normal.x *= -1;
	b2_normal.y *= -1;
	b2->velocity.x = b2_normal.x + b2_tangent.x;
	
	return;
}

void clear_ball(ball *b){
		fill_circle(b->position.x, b->position.y, b->radius, BLACK);
}

void update_position(ball *b, uint32_t color){
		b->position.x += b->velocity.x;
		b->position.y += b->velocity.y;
		fill_circle(b->position.x, b->position.y, b->radius, color);
}

int main(){
	buffer_init();
	clear_buffer();

	ball b;
	b.radius = 30;
	b.position.x = 1000;
	b.position.y = 1000;
	b.velocity.x = 3;
	b.velocity.y = 4;

	ball c;
	c.radius = 30;
	c.position.x = 1040;
	c.position.y = 300;
	c.velocity.x = 3;
	c.velocity.y = -2;

	ball d;
	d.radius = 30;
	d.position.x = 340;
	d.position.y = 600;
	d.velocity.x = 5;
	d.velocity.y = 2;

	int i = 100000;
	while (i--){
		clear_ball(&b);
		clear_ball(&c);
		clear_ball(&d);
		wall_collision(&b);
		wall_collision(&c);
		wall_collision(&d);
		update_position(&b, 0xffaa33);
		update_position(&c, 0x3366ff);
		update_position(&d, 0xff2233);
		if (touching(b, c)){
			collision(&b, &c);
		}
		if (touching(b, d)){
			collision(&b, &d);
		}
		if (touching(d, c)){
			collision(&d, &c);
		}

		usleep(10000);
	}

	buffer_reset();
}
