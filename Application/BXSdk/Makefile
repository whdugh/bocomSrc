#For C++ Project
#============================================
#create by mawei
#2013-4-12
#============================================
MODULES = BX_SDK
PROGNAME = $(MODULES)

BUILDDATE="\"\\\"tdqj_`date +%Y%m%d%k%M%S`\\\"\""

INCLUDEDIR=  
CPPFLAG = -Wall -o2 $(INCLUDEDIR) -fPIC -Wno-deprecated -D_FILE_OFFSET_BITS=64
CPPFLAG += -DBUILDDATE=$(BUILDDATE)

TARGET = $(PROGNAME)
LIB_NAME = $(addprefix lib,$(PROGNAME))
SHARE_NAME = $(LIB_NAME).so
STATIC_NAME = $(LIB_NAME).a

LIBRARY_PATHS = 
SHARE_LIBS = pthread
STATIC_LIBS =  

#libMM.a libBase.a


LIBRARY = $(patsubst %, -l%, $(SHARE_LIBS))
LIBRARY_PATH = $(patsubst %, -L%, $(LIBRARY_PATHS))

CXX = g++
CC = gcc
AR = ar
ARFLAGS = -rc

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@$(CXX) $(CPPFLAG) -c $< -o $@
%.o:%.c
	@echo "Compile $@ ($<)."
	@$(CC) $(CPPFLAG) -c $< -o $@

SOURCE = $(wildcard *.c *.cpp)

OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

#Linking to execute file
#($TARGET):$(OBJS)
#	@echo "Linking to excute($@ : $(OBJS))."
#	@$(CXX)  $(CPPFLAG)  -o  $(TARGET) $(OBJS) $(LIBRARY_PATH) $(STATIC_LIBS)  $(LIBRARY)
 
# Linking to share ( dynamic ) library
$(STATIC_NAME): $(OBJS)	
	@echo "Archive to static library($@ [$(OBJS)])."
	@${AR} $(ARFLAGS) $@ $(OBJS) $(LIBRARY_PATH)
	cp -f $(STATIC_NAME) ../../MvsServer/lib/static/
#$(SHARE_NAME): $(OBJS)
#	@echo "Linking to shared library($@ [$(OBJS)])."	
#	@$(CXX) $(CPPFLAGS) -o $@ $(OBJS) -shared $(LIBRARY_PATH) $(LIBRARY) $(STATIC_LIBS)

.PHONY:clean
clean:
	@echo "clean ($(MODULES) $(OBJS))."
	@-rm -rf $(MODULES)
	@-rm -rf $(OBJS)
