CC = gcc
CFLAGS = -Wall -g 
SRCS = main.c draw.c life.c
OBJS = $(SRCS:.c=.o)
LDFLAGS = -lm
TARGET = life

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

#pattern rule for generating .o files from .c files
%.o:%.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


: $(TARGET)
	./$(TARGET)
