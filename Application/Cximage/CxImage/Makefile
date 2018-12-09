TARGET = libCxImage.a
CXX = g++
CC=gcc
CXXFLAGS = -Wall -O2 -fPIC 
OBJECTS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJECTS2 = $(patsubst %.c,%.o,$(wildcard *.c))

$(TARGET): $(OBJECTS) 
	$(OBJECTS2)
	$(AR) cru $@ $?
	$(RANLIB) $@
	cp -rf $(TARGET) ../../MvsServer/lib/
	chmod +x ../../MvsServer/lib/$(TARGET)
$(OBJECTS): %.o: %.cpp 
	$(CXX) -c -I../jasper/include $(CXXFLAGS) $< -o $@

$(OBJECTS2): %.o:  %.c
	$(CC) -c -I../jasper/include $(CXXFLAGS) $< -o $@
clean:
	rm -f $(TARGET) $(OBJECTS) $(OBJECTS2)

include ../Defs.make
