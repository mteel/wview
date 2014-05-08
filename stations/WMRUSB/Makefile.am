# Makefile - Oregon Scientific WMRUSB station daemon (USB HID)

# define the executable to be built
bin_PROGRAMS    = wviewd_wmrusb

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
wviewd_wmrusb_SOURCES     = \
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
		$(top_srcdir)/stations/common/stormRain.c \
		$(top_srcdir)/stations/common/parser.c \
		$(top_srcdir)/stations/common/usbhid.c \
		$(top_srcdir)/stations/WMRUSB/wmrusbinterface.c \
		$(top_srcdir)/stations/WMRUSB/wmrusbprotocol.c \
		$(top_srcdir)/common/sensor.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/common/windAverage.h \
		$(top_srcdir)/common/emailAlerts.h \
		$(top_srcdir)/common/beaufort.h \
		$(top_srcdir)/stations/common/computedData.h \
		$(top_srcdir)/stations/common/daemon.h \
		$(top_srcdir)/stations/common/station.h \
		$(top_srcdir)/stations/common/stormRain.h \
		$(top_srcdir)/stations/common/parser.h \
		$(top_srcdir)/stations/common/hidapi.h \
		$(top_srcdir)/stations/common/usbhid.h \
		$(top_srcdir)/stations/WMRUSB/wmrusbinterface.h \
		$(top_srcdir)/stations/WMRUSB/wmrusbprotocol.h

if DARWIN
wviewd_wmrusb_SOURCES	+= $(top_srcdir)/stations/common/hidapi-osx.c
wviewd_wmrusb_LDFLAGS	= -L$(prefix)/lib -L/usr/lib -L/usr/local/lib -framework IOKit -framework CoreFoundation
wviewd_wmrusb_LDADD		=
else
wviewd_wmrusb_SOURCES	+= $(top_srcdir)/stations/common/hidapi-linux.c
wviewd_wmrusb_LDFLAGS	= -L$(prefix)/lib -L/usr/lib -L/usr/local/lib
if FREEBSD
wviewd_wmrusb_LDADD		= -lusb
else
wviewd_wmrusb_LDADD		= -lusb-1.0
endif
endif
