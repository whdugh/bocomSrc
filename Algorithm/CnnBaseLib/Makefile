# Makefile tmeplate

#============================================
# Part 1: util variable definition
AR = ar
CC = icc
CXX = icpc

#CC = gcc
#CXX = g++
LD = ld
RM = rm

#============================================
CnnBaseLib_BUILDDATE="\"\\\"`date +%Y-%m-%d\ %k:%M:%S`\\\"\""
CnnBaseLib_SVNURL="\"\\\"`svn info | grep URL`\\\"\""
CnnBaseLib_SVNNUM="\"\\\"`svn info | grep Revision`\\\"\""
#============================================
# Part 2: compile flags
ARFLAG = r
CFLAG = -O2 -xHost -fPIC -DLINUX 
CFLAG+= -DCnnBaseLib_BUILDDATE=$(CnnBaseLib_BUILDDATE)
CFLAG+= -DCnnBaseLib_SVNURL=$(CnnBaseLib_SVNURL)
CFLAG+= -DCnnBaseLib_SVNNUM=$(CnnBaseLib_SVNNUM)
CPPFLAG = -O2 -xHost -fPIC -DLINUX 
CPPFLAG += -DCnnBaseLib_BUILDDATE=$(CnnBaseLib_BUILDDATE)
CPPFLAG += -DCnnBaseLib_SVNURL=$(CnnBaseLib_SVNURL)
CPPFLAG += -DCnnBaseLib_SVNNUM=$(CnnBaseLib_SVNNUM)
RMFLAG = -rf

#============================================
# Part 3: include files directory
#         library files directory
INCLUDE = -Iinclude -I/usr/local/opencv-2.3.1/include -I/usr/local/opencv-2.3.1/include/opencv -I/opt/ipp8.2/ipp/include
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
TARGET = ../MvsServer/lib/libCnnBaseLib.a

#============================================
# Part 5: compile process


$(TARGET): $(OBJ)
	$(AR) r $@ $(OBJ);
	chmod 777 $(TARGET);
	chmod -R 777 $(OBJ);
	echo "$(CnnBaseLib_BUILDDATE)";
	echo "$(CnnBaseLib_SVNURL)";
	echo "$(CnnBaseLib_SVNNUM)";


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
