CC      = cc
CXX     = c++
CFLAGS  = -g -O3 -Wall -MMD -MD
LIBS    = -lmosquitto
LDFLAGS = -g

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all:		MMDVM-CrossMode

MMDVM-CrossMode:	$(OBJS)
		$(CXX) $(OBJS) $(CFLAGS) $(LIBS) -o MMDVM-CrossMode

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<
-include $(DEPS)

MMDVM-CrossMode.o: GitVersion.h FORCE

.PHONY: GitVersion.h

FORCE:

clean:
		$(RM) MMDVM-CrossMode *.o *.d *.bak *~ GitVersion.h

install:
		install -m 755 MMDVM-CrossMode /usr/local/bin/

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif
