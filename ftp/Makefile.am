# Makefile - wviewftpd

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS    = wviewftpd

# define include directories
INCLUDES = 	-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-D_GNU_SOURCE

# define the sources
wviewftpd_SOURCES  = \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/ftp/ftp.c \
		$(top_srcdir)/ftp/ftpUtils.c \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/ftp/ftp.h \
		$(top_srcdir)/ftp/ftpUtils.h


# define libraries
wviewftpd_LDADD   = -lssl -lcurl

# define library directories
wviewftpd_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wviewftpd_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

