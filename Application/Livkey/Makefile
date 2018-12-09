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
CFLAG = -O2 -xHost -fPIC
CPPFLAG = -O2 -xHost -fPIC
RMFLAG = -rf

#============================================
# Part 3: include files directory
#         library files directory
INCLUDE =  -Iinclude
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
TARGET = ../MvsServer/lib/libLivKey.a

#============================================
# Part 5: compile process


$(TARGET): $(OBJ)
	$(AR) $(ARFLAG)  $@  $(OBJ)
	chmod 770 $(TARGET)

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
