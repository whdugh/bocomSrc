OBJS = drv.o
CC = gcc
CFLAGS = -O2 -fPIC -shared

#-Wall -ldl
libYUV:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o ../../libYUV.so -lm

drv.o : yuv_driver.c yuv_driver.h
	$(CC) $(CFLAGS) -c yuv_driver.c -o drv.o


clean:
		rm -rf *.o libYUV