TARGET = libjasper.a

OBJECTS = $(patsubst %.c,%.o,$(wildcard */*.c))

$(TARGET): $(OBJECTS)
	$(AR) cru $@ $?
	$(RANLIB) $@
	mv $(TARGET) ../lib/$(TARGET)
     
$(OBJECTS): %.o: %.c
	$(CC) -c -Iinclude $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

include ../Defs.make
