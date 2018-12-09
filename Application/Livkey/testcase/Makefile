CC= icpc -O2 -xHost
CCFLAGS= -Wall -O2 -DLINUX
INC = -I"../include" 

demos:test.cpp
	$(CC) $(CCFLAGS) -o testusbkey.out test.cpp -L../lib -lnewusbkey -L./ -lclave2 $(INC)


clean:
	rm -f testnewusbkey.out
