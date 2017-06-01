# This software was developed at the National Institute of Standards and
# Technology (NIST) and the Intelligence Advanced Research Projects Activity
# (IARPA) by employees of the Federal Government in the course of their
# official duties. Pursuant to title 17 Section 105 of the United States Code,
# this software is not subject to copyright protection and is in the public
# domain. NIST and IARPA assume no responsibility whatsoever for its use by
# other parties, and makes no guarantees, expressed or implied, about its
# quality, reliability, or any other characteristic.

.PHONY: all clean

LOCALLIB := ../lib
LOCALINC := ../../include
LOCALBIN := ../bin

CP := cp -f

OS := $(shell uname -s)
ifeq ($(OS),Darwin)
LIBNAME_EXT := dylib
else
LIBNAME_EXT := so
endif

LIBNAME_REGEX := "libn2n_[A-Za-z0-9]+_[0-9]+.$(LIBNAME_EXT)"

# Generate -l flag for the core library in LOCALLIB
ifeq ($(shell ls $(LOCALLIB) | grep -E $(LIBNAME_REGEX) | wc -l | awk '{print $$1}'),1)
        CORE_LIB := $(shell ls $(LOCALLIB) | grep -E $(LIBNAME_REGEX))
        LIB_ARGS := -l$(patsubst lib%.$(LIBNAME_EXT),%,$(CORE_LIB))
else
        ifeq ($(shell ls $(LOCALLIB) | grep -E $(LIBNAME_REGEX) | wc -l | awk '{print $$1}'),0)
                $(error No core library found in $(LOCALLIB). Please review naming conventions)
        else
                $(error More than one core library found in $(LOCALLIB). Please review naming conventions)
        endif
endif
ifeq ($(LIB_ARGS), )
        $(error You did not place your libraries in $(LOCALLIB))
endif
PARTICIPANT_LIB_OPT := -L$(LOCALLIB) $(LIB_ARGS) -Wl,-rpath,$(shell readlink -f $(LOCALLIB))

PROGRAMS := n2nv_version n2nv_makeTemplates n2nv_finalize n2nv_identStageOne n2nv_identStageTwo

DISPOSABLEFILES := $(PROGRAMS) *.o .gdb_history *.$(LIBNAME_EXT) *.a
DISPOSABLEDIRS := validation_output* *.dSYM $(LOCALBIN)

CXXFLAGS += -I. -g -std=c++11 -Wall -pedantic -I$(LOCALINC) -I/usr/local/include
LDFLAGS += $(PARTICIPANT_LIB_OPT)

LDFLAGS += -L/usr/local/lib -lbiomeval

CC := $(CXX)

all: $(PROGRAMS)
	mkdir -p $(LOCALBIN) && $(CP) $(PROGRAMS) $(LOCALBIN)

clean:
	$(RM) $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)

n2nv_version: n2nv_version.o
n2nv_makeTemplates: n2nv_makeTemplates.o n2nv_enumerations.o
n2nv_finalize: n2nv_finalize.o
n2nv_identStageOne: n2nv_identStageOne.o
n2nv_identStageTwo: n2nv_identStageTwo.o

