include ../../common/MPI.defs

OPTFLAGS    = $(DEFAULT_OPT_FLAGS)
#description: change above into something that is a decent optimization on you system
 
#uncomment any of the following flags (and change values) to change defaults
 
USERFLAGS     = 
#description: parameter to specify optional flags
UMMAPIO_PATH = ../../../ummap-io/build
UTIL_PATH    = ../../../build
EXTOBJS      = $(UTIL_PATH)/obj/util.o
LIBS         = -lummapio
LIBPATHS     = -L$(UMMAPIO_PATH)/lib
INCLUDEPATHS = -I$(UMMAPIO_PATH)/include -I$(UTIL_PATH)/include

ifeq (,$(findstring Cray,$(shell cc -V 2>&1)))
    LIBS     += -pthread -lrt
    # OPTFLAGS += -std=c99
endif
 
### End User configurable options ###
 
ifndef RESTRICT_KEYWORD
  RESTRICT_KEYWORD=0
endif
#description: the "restrict" keyword can be used on IA platforms to disambiguate  
#             data accessed through pointers (requires -restrict compiler flag)

ifndef VERBOSE
  VERBOSE=0
endif
#description: default diagnostic style is silent

VERBOSEFLAG = -DVERBOSE=$(VERBOSE)
RESTRICTFLAG= -DRESTRICT_KEYWORD=$(RESTRICT_KEYWORD)

OPTIONSSTRING="Make options:\n\
OPTION                 MEANING                                  DEFAULT\n\
RESTRICT_KEYWORD=0/1   disable/enable restrict keyword (aliasing) [0]  \n\
VERBOSE=0/1            omit/include verbose run information       [0]"

TUNEFLAGS   = $(VERBOSEFLAG) $(USERFLAGS) $(RESTRICTFLAG)
PROGRAM     = reduce
OBJS        = $(PROGRAM).o $(COMOBJS)
 
include ../../common/make.common
