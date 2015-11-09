CXX=gcc
CXXFLAGS=-std=c++11 -Wall -Wno-unknown-pragmas -O3 -pipe
LDFLAGS=-lstdc++

all: fme.out tags

clean:
	rm -fr *.o *.out tags

format:
	clang-format -i *.cpp

tags: *.cpp
	ctags *.cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
fme.out: fme.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
#-----------------------------------------------------------------------------
fme.o: fme.cpp
#-----------------------------------------------------------------------------
