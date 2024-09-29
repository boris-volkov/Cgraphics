CC = gcc
INC_DIR = ../include
CFLAGS = -Wall -g -I$(INC_DIR)
SRCS = collision.c ../draw/draw.c
OBJS = $(SRCS:.c=.o)
LDFLAGS = -lm
TARGET = ball

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
