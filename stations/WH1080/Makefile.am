# Makefile - Fine Offset WH1080 (and friends) station daemon (USB HID)

# define the executable to be built
bin_PROGRAMS	= wviewd_wh1080

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
wviewd_wh1080_SOURCES     = \
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
		$(top_srcdir)/stations/WH1080/wh1080Interface.c \
		$(top_srcdir)/stations/WH1080/wh1080Protocol.c \
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
		$(top_srcdir)/stations/WH1080/wh1080Interface.h \
		$(top_srcdir)/stations/WH1080/wh1080Protocol.h


if DARWIN
wviewd_wh1080_SOURCES	+= $(top_srcdir)/stations/common/hidapi-osx.c
wviewd_wh1080_LDFLAGS	= -L$(prefix)/lib -L/usr/lib -L/usr/local/lib -framework IOKit -framework CoreFoundation
wviewd_wh1080_LDADD	=
else
wviewd_wh1080_SOURCES	+= $(top_srcdir)/stations/common/hidapi-linux.c
wviewd_wh1080_LDFLAGS	= -L$(prefix)/lib -L/usr/lib -L/usr/local/lib
if FREEBSD
wviewd_wh1080_LDADD		= -lusb
else
wviewd_wh1080_LDADD		= -lusb-1.0
endif
endif
