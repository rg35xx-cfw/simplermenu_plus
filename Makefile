# Software Name
TARGET = simplermenu_plus

#CROSS_COMPILE = /opt/cdata/rg35xx/upstream/batocera.linux/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-

# Compiler
CC = g++
LINKER   = g++
CFLAGS = -fdata-sections -ffunction-sections -fPIC -flto -Wall
#-Ofast
LIBS += -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx -lz -lpthread -lm # -lasound

# You can use Ofast too but it can be more prone to bugs, careful.
CFLAGS += -g -Iinclude/
LDFLAGS = -Wl,--start-group $(LIBS) -Wl,--end-group -Wl,--as-needed -Wl,--gc-sections -flto

DEBUG = YES

SRCDIR = src
OBJDIR = obj
BINDIR = output
SOURCES := $(wildcard $(SRCDIR)/*.cpp)

OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(OBJECTS))

rm = rm -f

all: clean remove prepare $(BINDIR)/$(TARGET) $(OBJECTS)

prepare:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
