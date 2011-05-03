CC = gcc
OBJS = mosaic.o vector.o util.o
DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -DXP_UNIX $(DEBUG_FLAGS)
LIBS = -lgd -lm

mosaic : $(OBJS)
	$(CC) $(DEBUG_FLAG) -o mosaic $(OBJS) $(LIBS)

mosaic.o : mosaic.c
util.o : util.c
vector.o : vector.c

clean :
	rm $(OBJS) mosaic
