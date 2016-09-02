CFLAGS=-W -Wall -std=c99 -O2 -DNDEBUG
DEBUG_FLAG=-W -Wall -std=c99 -g

SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
DEBUG_OBJECTS=$(patsubst %.c,%.dbg.o,$(SOURCES))
DEPENDS=$(patsubst %.c,%.depends,$(SOURCES))
TARGET=dictionary
DEBUG_TARGET=dictionary.dbg
ASM=$(patsubst %.c,%.s,$(SOURCES))


all: $(TARGET)
	
debug: $(DEBUG_TARGET)

asm: $(ASM)

%.s: %.c
	$(CC) $(CFLAGS) -S -o $@ $<
	
%.dbg.o: %.c
	$(CC) $(DEBUG_FLAG) -c -o $@ $<
#	$(CC) $(CFLAGS) $(DEBUG_FLAG) -c -o $@ $<
	 
$(TARGET): $(TARGET).o $(OBJECTS)
$(DEBUG_TARGET): $(DEBUG_TARGET).o $(DEBUG_OBJECTS)

%.depends: %.c
	$(CC) -MM $(CFLAGS) $< > $@
	$(CC) -MM $(CFLAGS) $(DEBUG_FLAG) -MT $(patsubst %.c,%.dbg.o,$<) $< >> $@

.PHONY: clean all debug

clean:
	$(RM) $(OBJECTS) $(DEBUG_OBJECTS) $(TARGET) $(DEBUG_TARGET) $(DEPENDS) $(ASM)
-include $(DEPENDS)


