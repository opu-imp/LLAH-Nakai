CXX = g++ -Wall -O2 -g
CXXFLAGS = `pkg-config --cflags opencv`
LDFLAGS = `pkg-config --libs opencv`

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
