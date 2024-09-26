#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

int level = 5;

void cleanup(void){
	// here we will also release the buffer
	// and set the terminal back to normal mode
	// and free the font file
	printf("level: %d\n", level);
}

typedef struct {
	int a;
	int b;
	char symbol;
	int answer;
} question;

int randint(int a, int b){
	return rand()%(b-a+1) + a;
}

question addition (){ // returns the correct answer
	question q;
	q.a = randint(level-4, level*2);
	q.b = randint(level-4, level*2);
	q.answer = q.a + q.b;
	q.symbol = '+'; //this can be the ordinal of the better character from the font file esp division 
	return q;
}

question subtraction () {
	question q;
	q.b = 	randint( round( log2(level)), round(level*2));
	q.answer = randint( round( log2(level)), round(level*2));
	q.a = q.b + q.answer;
	q.symbol = '-';
	return q;
}

question multiplication () {
	question q;
	q.a = randint(round(level/3), round(level*2/3 + 3));
	q.b = randint(3,10 + round(log(level)/log(16)));
	q.answer = q.a*q.b;
	q.symbol = '*';
	return q;
}

question division() {
	question q;
	q.a = level*11;
	while (q.a >= level*10){
		q.answer = randint(floor(level/3),ceil(level/2));
		q.b = randint(2,ceil(level/2));
		q.a = q.b*q.answer;
	}
	q.symbol = '%';
	return q;
}

question (*functions[])() = {addition, subtraction, multiplication, division};

void print_question(question q){
	// based on length of a and b and answer, 
	// right justify everything 
	// for now just print it stupidly:
	printf("%4d\n%c%3d\n", q.a, q.symbol, q.b);
}

//a logistic scoring function
//time in seconds should be the input
void update_level(double time_taken){
    static double L = 3;
    static double  m = 3.8;
    static double  k = 1;
    static double  v = 0.3;
    level += (int) L / ( 1 + exp( k*( time_taken -m-log10(level)) ) ) + v;
}

void *timer(void *time){
	//int duration = *(int*)time;
	sleep(60);
	// instead of just sleeping this can also draw the status bar. 
	// kill the program, show the level
	if (kill(getpid(), SIGTERM) != 0)
		perror("failed to send SIGTERM");
	pthread_exit(NULL);
}

void handle_sigterm(int signum){
	exit(0);
}

int main(){
	if (atexit(cleanup) != 0){
		perror("failed to register cleanup");
	}

	if (signal(SIGTERM, handle_sigterm) == SIG_ERR){
		perror("failed to set up signal handler");
		return EXIT_FAILURE;
	}

	pthread_t timer_thread;
	pthread_create(&timer_thread, NULL, timer, NULL);

	struct timespec ts;

	srand((unsigned int) time(NULL));
	// main function getting a little long... loop below can be its own thing, maybe
	char line[16]; 		// buffer to hold user input
	int user_answer; 	// variable to hold the user's response
	int operation = 0;
	while (1){

		clock_gettime(CLOCK_MONOTONIC, &ts);
		//get time before
		double start_time = ts.tv_sec + ts.tv_nsec/1e9;


		question current = functions[(operation++)%4]();
		while (user_answer != current.answer){
			print_question(current);
			fgets(line, sizeof(line), stdin); // keep terminal in canonical mode?
			sscanf(line, "%d", &user_answer); // but turn off echo still, route buttons to my own big print
		}

		// get time after the question is answered
		clock_gettime(CLOCK_MONOTONIC, &ts);
		double end_time = ts.tv_sec + ts.tv_nsec/1e9;
		// update the level
		update_level(end_time - start_time);
		user_answer = -1;
	}

	pthread_join(timer_thread, NULL);
	return 0;
}

