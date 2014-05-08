# Makefile - FreeBSD

EXTRA_DIST = $(srcdir)/wview.sh

# define the scripts to be generated
noinst_SCRIPTS = wview

CLEANFILES = $(noinst_SCRIPTS)

wview: $(srcdir)/wview.sh
	echo "#!/usr/local/bin/bash" > wview
	echo "" >> wview
	echo "# add to the shared library search path" >> wview
	echo "export LD_LIBRARY_PATH=$(prefix)/lib:/usr/local/lib:/usr/lib" >> wview
	echo "CONF_DIRECTORY=$(sysconfdir)/wview" >> wview
	echo "RUN_DIRECTORY=$(localstatedir)/wview" >> wview
	echo "WVIEW_INSTALL_DIR=$(exec_prefix)/bin" >> wview
	cat $(srcdir)/wview.sh >> wview
	chmod ugo+x wview
