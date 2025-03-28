# Compiler to use
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Linker flags and libraries using pkg-config
LDFLAGS = $(shell pkg-config --cflags --libs dconf gtk4 glib-2.0 gio-2.0)

# Source directory
SRCDIR = src

# Object directory
OBJDIR = obj

# Binary output directory
BINDIR = .build

# Target executable name
TARGET = $(BINDIR)/wallpaper

# Source files
SOURCES = $(SRCDIR)/main.cpp

# Object files
OBJECTS = $(OBJDIR)/main.o

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SRCDIR)/main.cpp -o $(OBJDIR)/main.o

# Clean up
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Phony targets
.PHONY: all clean