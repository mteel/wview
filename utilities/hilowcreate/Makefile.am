# Makefile - hilowcreate

#define the executable to be built
bin_PROGRAMS = hilowcreate

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_UTILITIES=1 \
		-D_GNU_SOURCE

# define the sources
hilowcreate_SOURCES  = \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/windAverage.h \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/dbsqliteHiLow.c \
		$(top_srcdir)/common/windAverage.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/utilities/hilowcreate/hilowcreate.c

# define libraries
hilowcreate_LDADD   =

# define library directories
hilowcreate_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
hilowcreate_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

