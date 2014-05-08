# Makefile - sqlite2wlk

#define the executable to be built
bin_PROGRAMS = sqlite2wlk

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
sqlite2wlk_SOURCES  = \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/stations/VantagePro/dbfiles.h \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/stations/VantagePro/dbfiles.c \
		$(top_srcdir)/utilities/sqlite2wlk/sqlite2wlk.c

# define libraries
sqlite2wlk_LDADD   =

# define library directories
sqlite2wlk_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
sqlite2wlk_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

