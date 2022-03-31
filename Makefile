CC     := g++
INCDIR := include
CFLAGS := -I$(INCDIR)

SRCDIR := src
OBJDIR := build

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all clean

all: $(OBJ)
	$(CC) -o main $(OBJ)

# Compiling source files into object files.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -fPIC

$(OBJDIR):
	mkdir -p $@

# Cleanup build and remove the executable file from the top directory.

clean:
	rm -rf $(OBJDIR)