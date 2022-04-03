CC     := g++
INCDIR := include
CFLAGS := -I$(INCDIR)

SRCDIR := src
OBJDIR := build

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all clean

all: $(OBJ)
	$(CC) -std=c++11 -o main $(OBJ)

# Compiling source files into object files.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) -std=c++11 $(CFLAGS) -c $< -o $@ -g -fPIC

$(OBJDIR):
	mkdir -p $@

# Cleanup build and remove the executable file from the top directory.

clean:
	rm -rf $(OBJDIR)
