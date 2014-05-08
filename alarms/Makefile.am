# Makefile - wvalarmd

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS    = wvalarmd

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_WVALARMD

# define the sources
wvalarmd_SOURCES  = \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/datafeed.c \
		$(top_srcdir)/common/datafeed.h \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/alarms/alarms.c \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/alarms/alarms.h


# define libraries
wvalarmd_LDADD   =

# define library directories
wvalarmd_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wvalarmd_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

