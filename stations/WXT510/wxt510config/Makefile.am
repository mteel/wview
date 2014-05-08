# Makefile - wxt510config

#define the executable to be built
bin_PROGRAMS = wxt510config

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(top_srcdir)/stations/common \
		-I$(top_srcdir)/stations/WXT510 \
		-I$(prefix)/include \
		-D_WXT510_CONFIG_ONLY \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-D_GNU_SOURCE

# define the sources
wxt510config_SOURCES  = \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/emailAlerts.h \
		$(top_srcdir)/stations/common/serial.h \
		$(top_srcdir)/stations/common/ethernet.h \
		$(top_srcdir)/stations/common/daemon.h \
		$(top_srcdir)/stations/common/parser.h \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/emailAlerts.c \
		$(top_srcdir)/stations/common/serial.c \
		$(top_srcdir)/stations/common/ethernet.c \
		$(top_srcdir)/stations/common/parser.c \
		$(top_srcdir)/stations/WXT510/nmea0183.h \
		$(top_srcdir)/stations/WXT510/wxt510Interface.h \
		$(top_srcdir)/stations/WXT510/nmea0183.c \
		$(top_srcdir)/stations/WXT510/wxt510Interface.c \
		$(top_srcdir)/stations/WXT510/wxt510config/wxt510config.c

# define libraries
wxt510config_LDADD   =

# define library directories
wxt510config_LDFLAGS = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wxt510config_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

