# Makefile tmeplate

#============================================
# Part 1: util variable definition
AR = ar
CC = icc
CXX = icpc
LD = ld
RM = rm


#============================================
# Part 2: compile flags
ARFLAG = r
CFLAG = -O2 -xHost -fPIC -DLINUX
CPPFLAG = -O2 -xHost -fPIC -DLINUX
RMFLAG = -rf

#============================================
# Part 3: include files directory
#         library files directory
INCLUDE = -Iinclude -I/usr/local/opencv-2.3.1/include -I/usr/local/opencv-2.3.1/include/opencv -I../MvsServer/IPP 
INCLUDE +=-I../LineSeg/include -I../Sift/include
LIBRARY = 

#============================================
# Part 4: project staff

# source files
CSRC = $(wildcard source/*.c) # ./*.c
CPPSRC = $(wildcard source/*.cpp) # ./*.cpp
SRC = $(CSRC) $(CPPSRC)

# object files
COBJ = $(patsubst %.c, %.o, $(CSRC))
CPPOBJ = $(patsubst %.cpp, %.o, $(CPPSRC))
OBJ = $(COBJ) $(CPPOBJ)

# target
TARGET = ../MvsServer/lib/libHog.a

#============================================
# Part 5: compile process


$(TARGET): $(OBJ)
	$(AR) r $@ $(OBJ)
	chmod 777 $(TARGET)
	chmod -R 777 $(OBJ)

$(COBJ): %.o: %.c
	$(CC) $(CFLAG) -c $< -o $@ $(INCLUDE) $(LIBRARY)

$(CPPOBJ): %.o: %.cpp
	$(CXX) $(CPPFLAG) -c $< -o $@ $(INCLUDE) $(LIBRARY)


#============================================
# Part 6: clean
clean:
	$(RM) $(RMFLAG) $(OBJ) $(TARGET)

cleanall:
	$(RM) $(RMFLAG) $(OBJ) $(TARGET) $(SRC)
