# Makefile - Vaisala WXT510 station daemon

# define the wxt510config target
SUBDIRS = wxt510config

# define the executable to be built
bin_PROGRAMS    = wviewd_wxt510

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(top_srcdir)/stations/common \
		-I$(prefix)/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_WVIEWD

# define the sources
wviewd_wxt510_SOURCES     = \
		$(top_srcdir)/common/sensor.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/dbsqliteHiLow.c \
		$(top_srcdir)/common/windAverage.c \
		$(top_srcdir)/common/emailAlerts.c \
		$(top_srcdir)/stations/common/computedData.c \
		$(top_srcdir)/stations/common/daemon.c \
		$(top_srcdir)/stations/common/station.c \
		$(top_srcdir)/stations/common/serial.c \
		$(top_srcdir)/stations/common/ethernet.c \
		$(top_srcdir)/stations/common/stormRain.c \
		$(top_srcdir)/stations/common/parser.c \
		$(top_srcdir)/stations/WXT510/wxt510Interface.c \
		$(top_srcdir)/stations/WXT510/nmea0183.c \
		$(top_srcdir)/common/sensor.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/common/windAverage.h \
		$(top_srcdir)/common/emailAlerts.h \
		$(top_srcdir)/common/beaufort.h \
		$(top_srcdir)/stations/common/computedData.h \
		$(top_srcdir)/stations/common/daemon.h \
		$(top_srcdir)/stations/common/station.h \
		$(top_srcdir)/stations/common/serial.h \
		$(top_srcdir)/stations/common/ethernet.h \
		$(top_srcdir)/stations/common/stormRain.h \
		$(top_srcdir)/stations/common/parser.h \
		$(top_srcdir)/stations/WXT510/wxt510Interface.h \
		$(top_srcdir)/stations/WXT510/nmea0183.h

# define libraries
wviewd_wxt510_LDADD       =

# define library directories
wviewd_wxt510_LDFLAGS     = -L$(prefix)/lib -L$(prefix)/usr/lib -L/usr/lib

if CROSSCOMPILE
wviewd_wxt510_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

