###############################################################################
#                                                                             #
#  Makefile for the alarm datafeed client example                                            #
#                                                                             #
#  Name                 Date           Description                            #
#  -------------------------------------------------------------------------  #
#  MS Teel              01/10/02       Initial Creation                       #
#                                                                             #
###############################################################################
#  Define the C compiler and its options
CC			= gcc
CC_OPTS			= -Wall -g -O2
SYS_DEFINES		= \
			-D_GNU_SOURCE

#  Define the Linker and its options
LD			= gcc
LD_OPTS			=

#  Define the Library creation utility and it's options
LIB_EXE			= ar
LIB_EXE_OPTS	= -rv

#  Define the dependancy generator
DEP			= gcc -MM

################################  R U L E S  ##################################
#  Generic rule for c files
%.o: %.c
	@echo "Building   $@"
	$(CC) $(CC_OPTS) $(SYS_DEFINES) $(DEFINES) $(INCLUDES) -c $< -o $@


#  Define some general usage vars
#  Libraries
LIBS			= \
			-lc \
			-lz \
			-lrad \
			-lsqlite3

LIBPATH 		= -L/usr/local/lib -L/usr/lib

#  Declare build defines
DEFINES			= \
			-D_DEBUG

#  Any build defines listed above should also be copied here
INCLUDES		= \
			-I. \
			-I../../common \
			-I/usr/local/include

########################### T A R G E T   I N F O  ############################
EXE_IMAGE		= datafeedClient

TEST_OBJS		= \
			../../common/datafeed.o \
			./datafeedClient.o


#########################  E X P O R T E D   V A R S  #########################


################################  R U L E S  ##################################

$(EXE_IMAGE):	$(TEST_OBJS)
	@echo "Linking $@..."
	$(LD) $(LD_OPTS) $(LIBPATH) -o $@ $(TEST_OBJS) $(LIBS)


all: clean $(EXE_IMAGE)


#  Cleanup rules...
clean: 
	rm -rf \
	$(EXE_IMAGE) \
	$(TEST_OBJS)

