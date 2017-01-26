# This software was developed at the National Institute of Standards and
# Technology (NIST) and the Intelligence Advanced Research Projects Activity
# (IARPA) by employees of the Federal Government in the course of their
# official duties. Pursuant to title 17 Section 105 of the United States Code,
# this software is not subject to copyright protection and is in the public
# domain. NIST and IARPA assume no responsibility whatsoever for its use by
# other parties, and makes no guarantees, expressed or implied, about its
# quality, reliability, or any other characteristic.

.PHONY: all clean

COMMONINCOPT := -I/usr/local/include -I.
COMMONLIBOPT := -L/usr/local/lib -lbiomeval

LOCALINC := ../include
LOCALLIB := ../lib

include ../common.mk

CXXFLAGS += -O3

SOURCE := nullimpl.cpp
OBJECT := $(SOURCE:%.cpp=%.o)

#
# Get identifier and revision number from the source code directly.
#
IDENTIFIER := $(shell grep -m 2 identifier $(SOURCE) | grep -v '&' | sed 's/.*=[[:space:]]*"\([[:alnum:]]\{1,\}\)";/\1/')
REVISION := $(shell grep -m 2 revision $(SOURCE)| grep -v '&' | sed 's/.*=[[:space:]]*\([[:digit:]]\{1,\}\)[[:space:]]*;/\1/')

LIBRARY := libn2n_$(IDENTIFIER)_$(REVISION)

all: $(LIBRARY)

$(LIBRARY): $(OBJECT)
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)
ifeq ($(OS),Darwin)
	$(CXX) $(CXXFLAGS) -dynamiclib $(OBJECT) -o $(LIBRARY).dylib $(LDFLAGS)
	$(CP) $(LIBRARY).dylib $(LOCALLIB)
else
	$(CXX) $(CXXFLAGS) -shared $(OBJECT) -o $(LIBRARY).so $(LDFLAGS)
	$(CP) $(LIBRARY).so $(LOCALLIB)
endif

clean:
	$(RM) $(LIBRARY).* $(LOCALLIB)/$(LIBRARY).* $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
