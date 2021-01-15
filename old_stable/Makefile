# debug
CXX = g++ -Wall -O2 -g -DLINUX
# release
#CXX = g++ -Wall -O2 -DLINUX

CXXFLAGS = `pkg-config --cflags opencv`
LDFLAGS = `pkg-config --libs opencv`

PACKAGE = llahdoc

SRCS = llahdoc.c nn5.c auto_connect.c disc.c gj.c houghl.c proj4p.c block.c corres.c f_points.c hash.c projrecov.c cluster.c cr.c gencomb.c hist.c perminv.c qsort.c proctime.c score.c init.c prop.c nstr.c sock_sv.c nears.c daytime.c ncr.c inacr2.c fpath.c hlist2.c

HEADS = auto_connect.h block.h cluster.h corres.h cr.h dirs.h disc.h f_points.h gencomb.h gj.h hash.h hist.h houghl.h nn5.h perminv.h proctime.h proj4p.h projrecov.h score.h init.h prop.h nstr.h extern.h def_general.h sock_sv.h nears.h daytime.h ncr.h inacr2.h fpath.h hlist2.h


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
