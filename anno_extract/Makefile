CXX = g++ -Wall -O2 -g
CXXFLAGS = `pkg-config --cflags opencv`
LDFLAGS = `pkg-config --libs opencv`

PACKAGE = annex

SRCS = annex.c annex_st.c annex_diff.c transform.c clpoints.c hash.c disc.c gencomb.c block.c hist.c nn5j.c cr.c proctime.c estimate.c nstr.c nimg.c

HEADS = annex.h annex_st.h annex_diff.h transform.h clpoints.h def_general.h hash.h disc.h gencomb.h block.h hist.h dirs.h nn5j.h cr.h proctime.h estimate.h nstr.h nimg.h

OBJS = $(SRCS:.c=.o)
FILES = Makefile $(HEADS) $(SRCS)

all:	$(PACKAGE)

$(PACKAGE):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PACKAGE) $(OBJS)
.c.o:
	$(CXX) $(CXXFLAGS) -c $<
#$(OBJS):	$(SRCS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(SRCS)
$(OBJS): $(HEADS) Makefile

clean:	
	rm -f $(OBJS)
