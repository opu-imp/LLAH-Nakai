CXX = g++ -Wall -O2 -g -std=c++17
# Use OpenCV 5 if pkg-config finds it, otherwise OpenCV 4.
# To choose explicitly: make OPENCV_PC=opencv4 (or OPENCV_PC=opencv5)
OPENCV_PC ?= $(shell pkg-config --exists opencv5 && echo opencv5 || echo opencv4)
CXXFLAGS = `pkg-config --cflags $(OPENCV_PC)`
LDFLAGS = `pkg-config --libs $(OPENCV_PC)`

PACKAGE = acr2

SRCS = acr2.c hist.c gencomb.c cr.c
HEADS = acr2.h hash.h cvtest.h disc.h hist.h

OBJS = $(SRCS:.c=.o)
FILES = Makefile $(HEADS) $(SRCS)

all:	$(PACKAGE)

$(PACKAGE):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PACKAGE) $(OBJS)
.c.o:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $<
#$(OBJS):	$(SRCS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(SRCS)

$(OBJS): $(HEADS) Makefile
clean:	
	rm -f $(OBJS)
