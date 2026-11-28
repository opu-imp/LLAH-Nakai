# debug
#CXX = g++ -Wall -O2 -g -std=c++17
# release
CXX = g++ -Wall -O2 -std=c++17

# Use OpenCV 5 if pkg-config finds it, otherwise OpenCV 4.
# To choose explicitly: make OPENCV_PC=opencv4 (or OPENCV_PC=opencv5)
OPENCV_PC ?= $(shell pkg-config --exists opencv5 && echo opencv5 || echo opencv4)
CXXFLAGS = `pkg-config --cflags $(OPENCV_PC)`
LDFLAGS = `pkg-config --libs $(OPENCV_PC)`

PACKAGE = llahmos

SRCS = llahmos.c nn5.c auto_connect.c disc.c gj.c houghl.c proj4p.c block.c corres.c f_points.c hash.c projrecov.c cluster.c cr.c gencomb.c hist.c perminv.c qsort.c proctime.c score.c init.c prop.c nstr.c sock_sv.c nears.c daytime.c ncr.c inacr2.c fpath.c hlist2.c llahdoc.c nimg.c

HEADS = llahmos.h auto_connect.h block.h cluster.h corres.h cr.h dirs.h disc.h f_points.h gencomb.h gj.h hash.h hist.h houghl.h nn5.h perminv.h proctime.h proj4p.h projrecov.h score.h init.h prop.h nstr.h extern.h def_general.h sock_sv.h nears.h daytime.h ncr.h inacr2.h fpath.h hlist2.h nimg.h


OBJS = $(SRCS:.c=.o)
FILES = Makefile $(HEADS) $(SRCS)

all:	$(PACKAGE)

$(PACKAGE):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PACKAGE) $(OBJS)
.c.o:
	$(CXX) $(CXXFLAGS)  -c $<

#$(OBJS):	$(SRCS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(SRCS)
$(OBJS): $(HEADS) Makefile

clean:	
	rm -f $(OBJS)
