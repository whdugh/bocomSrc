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
INCLUDE = -Iinclude -I/usr/local/opencv-2.3.1/include -I/usr/local/opencv-2.3.1/include/opencv -I/opt/ipp8.2/ipp/include -I../Sift/include -I../LineSeg/include 
INCLUDE += -I../NewColorReg/include -I../NewCarnumlib/include -I../Tsai/include -I../Hog/include -I../Surf/include 
INCLUDE += -I../VibeModel/include -I../CTinyXml/include -I../NoPlateVehicle/include -I../PeoCarDetect/include 
LIBRARY =

#============================================
# Part 4: project staff

# source files
CSRC = $(wildcard source/*.c)
CPPSRC = $(wildcard source/*.cpp)
SRC = $(CSRC) $(CPPSRC)

# object files
COBJ = $(patsubst %.c, %.o, $(CSRC))
CPPOBJ = $(patsubst %.cpp, %.o, $(CPPSRC))
OBJ = $(COBJ) $(CPPOBJ)

# target
TARGET = ../MvsServer/lib/libFindTarget.a

#============================================
# Part 5: compile process

$(TARGET): $(OBJ)
	#$(CXX) $(CPPFLAG) -shared -o $@ -lc $(OBJ) $(INCLUDE) $(LIBRARY)
	$(AR) r $@ $(OBJ)
	chmod 777 $(TARGET)

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
