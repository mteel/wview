# Makefile - wviewconfig/wviewhtmlconfig/wviewcleardata

EXTRA_DIST = $(srcdir)/wviewconfig.sh $(srcdir)/wviewhtmlconfig.sh $(srcdir)/wviewcleardata.sh

# define the scripts to be generated
bin_SCRIPTS = wviewconfig wviewhtmlconfig wviewcleardata

CLEANFILES = $(bin_SCRIPTS)

wviewconfig: $(srcdir)/wviewconfig.sh
	rm -f wviewconfig
	echo "#!/bin/sh" > wviewconfig
	echo "WVIEW_CONF_DIR=$(sysconfdir)/wview" >> wviewconfig
	echo "WVIEW_DATA_DIR=$(localstatedir)/wview" >> wviewconfig
	cat $(srcdir)/wviewconfig.sh >> wviewconfig
	chmod ugo+x wviewconfig

wviewhtmlconfig: $(srcdir)/wviewhtmlconfig.sh
	rm -f wviewhtmlconfig
	echo "#!/bin/sh" > wviewhtmlconfig
	echo "WVIEW_CONF_DIR=$(sysconfdir)/wview" >> wviewhtmlconfig
	echo "WVIEW_DATA_DIR=$(localstatedir)/wview" >> wviewhtmlconfig
	echo "WVIEWCONFIG=$(exec_prefix)/bin/wviewconfig" >> wviewhtmlconfig
	cat $(srcdir)/wviewhtmlconfig.sh >> wviewhtmlconfig
	chmod ugo+x wviewhtmlconfig

wviewcleardata: $(srcdir)/wviewcleardata.sh
	rm -f wviewcleardata
	echo "#!/bin/sh" > wviewcleardata
	echo "WVIEW_CONF_DIR=$(sysconfdir)/wview" >> wviewcleardata
	echo "WVIEW_DATA_DIR=$(localstatedir)/wview" >> wviewcleardata
	cat $(srcdir)/wviewcleardata.sh >> wviewcleardata
	chmod ugo+x wviewcleardata

