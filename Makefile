TARGET=traka
CC=clang
CFLAGS=-std=c11 -Wall -Wextra -O2 -fsanitize=undefined,address `sdl2-config --cflags`
CFLAGS+=-framework OpenGL 
BUILDDIR=build
SRCDIR=src
LIBS=`sdl2-config --libs`

SOURCES=vendors/nanovg/nanovg.c src/main.c
SOURCES+=$(shell find $(SRCDIR) -name '*.c')
OBJECTS=$(SOURCES:%=$(BUILDDIR)/%.o)
INCLUDES=$(shell find $(SRCDIR) -type d)
DEPENDS= $(OBJECTS:.o=.d)

$(BUILDDIR)/vendors/nanovg/%.c.o: vendors/nanovg/nanovg/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR)/src/%.c.o: src/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

debug: 
	$(info $$var is [${SOURCES}])
	$(info $$var is [${OBJECTS}])
	$(info $$var is [${INCLUDES}])
	$(info $$var is [${DEPENDS}])

run: $(BUILDDIR)/$(TARGET)
	./build/$(TARGET) --debug

.PHONY: clean
clean: 
	rm -r $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(BUILDDIR)/roupiji 

-include $(DEPENDS)
