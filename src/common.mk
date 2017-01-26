# This software was developed at the National Institute of Standards and
# Technology (NIST) and the Intelligence Advanced Research Projects Activity
# (IARPA) by employees of the Federal Government in the course of their
# official duties. Pursuant to title 17 Section 105 of the United States Code,
# this software is not subject to copyright protection and is in the public
# domain. NIST and IARPA assume no responsibility whatsoever for its use by
# other parties, and makes no guarantees, expressed or implied, about its
# quality, reliability, or any other characteristic.

DISPOSABLEFILES = *.o .gdb_history
DISPOSABLEDIRS  = *.dSYM

CXXFLAGS := -std=c++11 -Wall -pedantic -fPIC -I$(LOCALINC) $(COMMONINCOPT)
LDFLAGS := -L$(LOCALLIB) $(COMMONLIBOPT)
CP := cp -f

OS := $(shell uname -s)
ifeq ($(OS),Darwin)
	OSX_MAJOR = $(shell sw_vers -productVersion | cut -d. -f1)
	OSX_MINOR = $(shell sw_vers -productVersion | cut -d. -f2)
	LDFLAGS += -Wl,-macosx_version_min,$(OSX_MAJOR).$(OSX_MINOR)
endif


