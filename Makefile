CC      = cc
CXX     = c++
CFLAGS  = -g -O3 -Wall
LIBS    =
LDFLAGS = -g

OBJECTS =	BPTC19696.o CRC.o Conf.o CrossMode.o Data.o DMRLookup.o DMRLC.o DMRNetwork.o DStarNetwork.o FMNetwork.o Golay24128.o Hamming.o \
			Log.o M17Network.o Network.o NXDNConvolution.o NXDNCRC.o NXDNFACCH1.o NXDNLayer3.o NXDNLICH.o NXDNLookup.o NXDNNetwork.o NXDNSACCH.o P25Network.o RS129.o StopWatch.o Thread.o Timer.o Transcoder.o \
			UARTController.o UDPSocket.o Utils.o YSFConvolution.o YSFFICH.o YSFNetwork.o YSFPayload.o

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
