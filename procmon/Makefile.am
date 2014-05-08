# Makefile - wvpmond

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS    = wvpmond

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_WVPMOND

# define the sources
wvpmond_SOURCES  = \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/procmon/procmon.c \
		$(top_srcdir)/procmon/procmonStates.c \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/procmon/procmon.h


# define libraries
wvpmond_LDADD   =

# define library directories
wvpmond_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wvpmond_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

