CXX = g++ -Wall -O2 -g -std=c++17
# Use OpenCV 5 if pkg-config finds it, otherwise OpenCV 4.
# To choose explicitly: make OPENCV_PC=opencv4 (or OPENCV_PC=opencv5)
OPENCV_PC ?= $(shell pkg-config --exists opencv5 && echo opencv5 || echo opencv4)
CXXFLAGS = `pkg-config --cflags $(OPENCV_PC)`
LDFLAGS = `pkg-config --libs $(OPENCV_PC)`

PACKAGE = annex

SRCS = annex.c annex_st.c annex_diff.c transform.c clpoints.c hash.c disc.c gencomb.c block.c hist.c nn5j.c cr.c proctime.c estimate.c nstr.c nimg.c

HEADS = annex.h annex_st.h annex_diff.h transform.h clpoints.h def_general.h hash.h disc.h gencomb.h block.h hist.h dirs.h nn5j.h cr.h proctime.h estimate.h nstr.h nimg.h

OBJS = $(SRCS:.c=.o)
FILES = Makefile $(HEADS) $(SRCS)

all:	$(PACKAGE) acr2_build

acr2_build:
	$(MAKE) -C acr2

$(PACKAGE):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PACKAGE) $(OBJS)
.c.o:
	$(CXX) $(CXXFLAGS) -c $<
#$(OBJS):	$(SRCS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(SRCS)
$(OBJS): $(HEADS) Makefile

clean:	
	rm -f $(OBJS)
	$(MAKE) -C acr2 clean
