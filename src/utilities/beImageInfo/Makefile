# This software was developed at the National Institute of Standards and
# Technology (NIST) and the Intelligence Advanced Research Projects Activity
# (IARPA) by employees of the Federal Government in the course of their
# official duties. Pursuant to title 17 Section 105 of the United States Code,
# this software is not subject to copyright protection and is in the public
# domain. NIST and IARPA assume no responsibility whatsoever for its use by
# other parties, and makes no guarantees, expressed or implied, about its
# quality, reliability, or any other characteristic.

.PHONY: all clean

FILE=beImageInfo

BIOMEVAL_PATH=/usr/local
CXXFLAGS += -std=c++11 -O3 -Wall -pedantic -I$(BIOMEVAL_PATH)/include
LDFLAGS += -L$(BIOMEVAL_PATH)/lib -lbiomeval -Wl,-rpath,$(BIOMEVAL_PATH)/lib

all: $(FILE)

$(FILE): $(FILE).o
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(FILE).o $(FILE)
