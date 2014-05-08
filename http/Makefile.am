# Makefile - wvhttpd

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS    = wvhttpd

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-I/usr/include \
		-I/usr/local/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_HTTP

# define the sources
wvhttpd_SOURCES  = \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/sensor.h \
		$(top_srcdir)/common/sensor.c \
		$(top_srcdir)/http/http.c \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/http/http.h


# define libraries
wvhttpd_LDADD     = -lssl -lcurl

# define library directories
wvhttpd_LDFLAGS   = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wvhttpd_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

