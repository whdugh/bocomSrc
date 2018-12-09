OBJS = capture.o
CC = gcc
CFLAGS = -O2 

#-Wall -ldl
libCapture.a:$(OBJS)
	ar r ../../lib/libCapture.a $(OBJS) 
	chmod +x ../../lib/libCapture.a
capture.o : capture.c capture.h
	$(CC) $(CFLAGS) -c capture.c -o capture.o

clean:
		rm -rf *.o libCapture.a
