# Makefile - arc_le2be

#define the executable to be built
bin_PROGRAMS = arc_le2be

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(top_srcdir)/stations/VantagePro \
		-I$(top_srcdir)/utilities \
		-I$(prefix)/include \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_UTILITIES \
		-D_GNU_SOURCE

# define the sources
arc_le2be_SOURCES  = \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/stations/VantagePro/dbfiles.h \
		$(top_srcdir)/utilities/wvutilities.h \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/stations/VantagePro/dbfiles.c \
		$(top_srcdir)/utilities/wvutilities.c \
		$(top_srcdir)/utilities/archive-le2be/arc_le2be.c

# define libraries
arc_le2be_LDADD   =

# define library directories
arc_le2be_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
arc_le2be_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

