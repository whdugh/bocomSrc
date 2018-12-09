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
INCLUDE = -Iinclude
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
SHATESTOBJ = shatest.o
TARGETOBJ = $(filter-out $(SHATESTOBJ), $(OBJ))

# target
TARGET = ../MvsServer/lib/libSha.a
#SHATEST = shatest
#============================================
# Part 5: compile process

all: $(TARGET)# $(SHATEST)
.PHONY: all

$(TARGET): $(TARGETOBJ)
#	$(CXX) $(CPPFLAG) -shared -o $@ -lc $(TARGETOBJ) $(INCLUDE) $(LIBRARY)
	$(AR) r $@ $(TARGETOBJ)
	chmod 770 $(TARGET)

#$(SHATEST): $(OBJ)
#	$(CXX) $(CPPFLAG) $(OBJ) -o $@ $(INCLUDE) $(LIBRARY)

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
