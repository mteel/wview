# Makefile - wview-mysql-export

EXTRA_DIST = \
	$(srcdir)/wview-mysql-export.sh \
	$(srcdir)/wview-mysql-create.sh \
	$(srcdir)/wview-pgsql-export.sh \
	$(srcdir)/wview-pgsql-create.sh

# define the scripts to be generated
bin_SCRIPTS = wview-mysql-export wview-mysql-create wview-pgsql-export wview-pgsql-create

CLEANFILES = $(bin_SCRIPTS)

wview-mysql-export: $(srcdir)/wview-mysql-export.sh
	rm -f wview-mysql-export
	echo "#!/bin/sh" > wview-mysql-export
	echo "WVIEW_CONF_DIR=$(sysconfdir)/wview" >> wview-mysql-export
	echo "WVIEW_DATA_DIR=$(localstatedir)/wview" >> wview-mysql-export
	cat $(srcdir)/wview-mysql-export.sh >> wview-mysql-export
	chmod ugo+x wview-mysql-export

wview-mysql-create:  $(srcdir)/wview-mysql-create.sh
	rm -f wview-mysql-create
	echo "#!/bin/sh" > wview-mysql-create
	cat $(srcdir)/wview-mysql-create.sh >> wview-mysql-create
	chmod ugo+x wview-mysql-create

wview-pgsql-export: $(srcdir)/wview-pgsql-export.sh
	rm -f wview-pgsql-export
	echo "#!/bin/sh" > wview-pgsql-export
	echo "WVIEW_CONF_DIR=$(sysconfdir)/wview" >> wview-pgsql-export
	echo "WVIEW_DATA_DIR=$(localstatedir)/wview" >> wview-pgsql-export
	cat $(srcdir)/wview-pgsql-export.sh >> wview-pgsql-export
	chmod ugo+x wview-pgsql-export

wview-pgsql-create:  $(srcdir)/wview-pgsql-create.sh
	rm -f wview-pgsql-create
	echo "#!/bin/sh" > wview-pgsql-create
	cat $(srcdir)/wview-pgsql-create.sh >> wview-pgsql-create
	chmod ugo+x wview-pgsql-create
