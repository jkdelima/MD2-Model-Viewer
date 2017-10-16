######################
## Generic Makefile ##
######################
# The following Makefile determines dependencies, compiles, and links all .cpp
# files found in the current directory into a single executable.

## Output exectable name
MAIN = app

## Files of interest.
# Use the wildcard to grab all .cpp files. Once built, select the associated 
# .o object files. Similarly, once build, select the associated .d files.
# .d files are temporary files created to establish file dependency
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)

## Linking flags, includes libraries used
LDFLAGS = -lGL -lGLU -lglut -L/usr/lib64 -lglfw -lGLEW

## Compilation flags
# -W -Wall      Requests all warnings be output to the console; -W prints extra
# -g            Puts debugging information for gdb into the object and 
#               executable file
# -pg puts      Puts profiling information for gprof into the object and
#               executable file. When the executable is run, it will produce 
#               a statistics file. gprof can then produce a listing indicating 
#               how long was spent in each function.
CXXFLAGS = -W -Wall -g

## Compiler to be used
CXX = g++

## Main target - to be run when calling 'make'
all: $(MAIN)

## Extra target
# Run using 'make depend' - creates dependencies
depend: $(DEPENDS)

## Extra target
# Run using 'make clean' - removes object and executable files
clean:
	rm -f *.o *.d $(MAIN)

## Defintion for building our executable. Relies on the OBJECTS
# $@ 		Refers to the rule's target. In this case 'app'
$(MAIN): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

## Definition for build our object files. Relies on the .cpp
# %  		Pattern matches. Creates a file with .o, given a file with .cpp
# $@ 		Refers to the pattern-matched target (ie. the .o file)
# %< 		Refers to the pattern-matched dependency (ie. the .cpp file)
%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

## Determine dependencies for each .cpp files.
# -M 		Creates a dependency directed acyclic graph, used by Makefiles
%.d: %.cpp
	$(CXX) -M $(CXXFLAGS) $< > $@

## Includes dependencies
include $(DEPENDS)

