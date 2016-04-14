# Manage the build of the application and run it
#
# Based on the following:
# http://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
#

CC := g++ -std=c++11
SRCDIR := src
BUILDDIR := build
TARGET := bin/runner

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall
LIB := #-pthread
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR)/*.o $(TARGET)"; $(RM) -r $(TARGET) $(BUILDDIR)/*.o
	@echo " $(RM) -r bin/*"; $(RM) -r bin/*

# Exclude runserver.cpp because that has its own main function (symbols collide)
TESTING_SOURCES := $(filter-out src/runserver.cpp, $(SOURCES))
tester:
	@echo " Testing...";
	@echo " $(CC) $(CFLAGS) $(TESTING_SOURCES) tests/tester.cpp $(INC) $(LIB) -o bin/tester"; \
	$(CC) $(CFLAGS) $(TESTING_SOURCES) tests/tester.cpp $(INC) $(LIB) -o bin/tester

.PHONY: clean
