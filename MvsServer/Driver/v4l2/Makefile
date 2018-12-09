OBJS = drv.o
CC = icc
CFLAGS = -O2 -xHost

#-Wall -ldl
libV4l2.a:$(OBJS)
	ar r libV4l2.a $(OBJS) 
	chmod +x libV4l2.a
	mv libV4l2.a ../../lib

drv.o : v4l2_driver.c v4l2_driver.h
	$(CC) $(CFLAGS) -c v4l2_driver.c -o drv.o

clean:
		rm -rf *.o ../../lib/libV4l2.a
