# Makefile - vpconfig

EXTRA_DIST = $(srcdir)/vpinstall.sh

# define the script to be generated
bin_SCRIPTS = vpinstall

#define the executable to be built
bin_PROGRAMS = vpconfig

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(top_srcdir)/stations/common \
		-I$(top_srcdir)/stations/VantagePro \
		-I$(prefix)/include \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-D_VP_CONFIG_ONLY \
		-D_GNU_SOURCE

# define the sources
vpconfig_SOURCES  = \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/stations/common/serial.h \
		$(top_srcdir)/stations/common/ethernet.h \
		$(top_srcdir)/stations/common/daemon.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/stations/common/serial.c \
		$(top_srcdir)/stations/common/ethernet.c \
		$(top_srcdir)/stations/VantagePro/vproInterface.h \
		$(top_srcdir)/stations/VantagePro/vproInterface.c \
		$(top_srcdir)/stations/VantagePro/vpconfig/vpconfig.c

# define libraries
vpconfig_LDADD   =

# define library directories
vpconfig_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
vpconfig_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

CLEANFILES = $(bin_SCRIPTS)

vpinstall: $(srcdir)/vpinstall.sh
	rm -f vpinstall
	echo "#!/bin/sh" > vpinstall
	echo "WVIEW_INSTALL_DIR=$(exec_prefix)/bin" >> vpinstall
	cat $(srcdir)/vpinstall.sh >> vpinstall
	chmod ugo+x vpinstall

