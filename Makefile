CC      = cc
CXX     = c++
CFLAGS  = -g -O3 -Wall -DHAVE_LOG_H
LIBS    =
LDFLAGS = -g

OBJECTS =	Conf.o CrossMode.o Data.o DStarNetwork.o FMNetwork.o Log.o M17Network.o Network.o StopWatch.o Thread.o Timer.o Transcoder.o UARTController.o UDPSocket.o Utils.o

all:		CrossMode

CrossMode:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o CrossMode

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

CrossMode.o: GitVersion.h FORCE

.PHONY: GitVersion.h

FORCE:

clean:
		$(RM) CrossMode *.o *.d *.bak *~ GitVersion.h

install:
		install -m 755 CrossMode /usr/local/bin/

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif
