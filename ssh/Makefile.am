# Makefile - wviewsshd

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS = wviewsshd

# define include directories
INCLUDES = 	-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-D_GNU_SOURCE

# define the sources
wviewsshd_SOURCES  = \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/ssh/ssh.c \
		$(top_srcdir)/ssh/sshUtils.c \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/ssh/ssh.h \
		$(top_srcdir)/ssh/sshUtils.h


# define libraries
wviewsshd_LDADD   =

# define library directories
wviewsshd_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wviewsshd_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

