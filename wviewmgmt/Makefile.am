# Makefile - wviewmgmt

EXTRA_DIST = \
	$(srcdir)/functions.php.sh \
	$(srcdir)/network_update.php \
	$(srcdir)/preload_station.php \
	$(srcdir)/services.php \
	$(srcdir)/password_protect.php \
	$(srcdir)/preload_system_status.php \
	$(srcdir)/sql_export.php \
	$(srcdir)/alarms.php \
	$(srcdir)/password_update.php \
	$(srcdir)/process_alarms.php \
	$(srcdir)/ssh.php \
	$(srcdir)/calibration.php \
	$(srcdir)/preload_alarms.php \
	$(srcdir)/process_calibration.php \
	$(srcdir)/station.php \
	$(srcdir)/cwop.php \
	$(srcdir)/preload_calibration.php \
	$(srcdir)/process_cwop.php \
	$(srcdir)/style.css \
	$(srcdir)/file_generation.php \
	$(srcdir)/preload_cwop.php \
	$(srcdir)/process_file_generation.php \
	$(srcdir)/system_status.php \
	$(srcdir)/ftp.php \
	$(srcdir)/preload_file_generation.php \
	$(srcdir)/process_ftp.php \
	$(srcdir)/wview-100x100.png \
	$(srcdir)/preload_ftp.php \
	$(srcdir)/process_http_services.php \
	$(srcdir)/wview-40x40.png \
	$(srcdir)/http_services.php \
	$(srcdir)/preload_http_services.php \
	$(srcdir)/process_services.php \
	$(srcdir)/wview.ico \
	$(srcdir)/preload_services.php \
	$(srcdir)/process_sql_export.php \
	$(srcdir)/wview_control.php \
	$(srcdir)/login.php \
	$(srcdir)/preload_sql_export.php \
	$(srcdir)/process_ssh.php \
	$(srcdir)/wview_upgrade.php \
	$(srcdir)/logout.php \
	$(srcdir)/preload_ssh.php \
	$(srcdir)/process_station.php \
	$(srcdir)/imgs/blueline.gif \
	$(srcdir)/imgs/calendar.png \
	$(srcdir)/imgs/email.png \
	$(srcdir)/imgs/header-norm.gif \
	$(srcdir)/imgs/phone.png \
	$(srcdir)/imgs/tip_small.png \
	$(srcdir)/imgs/black.png \
	$(srcdir)/imgs/blue.png \
	$(srcdir)/imgs/yellow.png \
	$(srcdir)/imgs/green.png \
	$(srcdir)/imgs/gray.png \
	$(srcdir)/imgs/red.png


# define the scripts to be generated
bin_SCRIPTS = functions.php

CLEANFILES = $(bin_SCRIPTS)

functions.php: $(srcdir)/functions.php.sh
	echo "<?php" > functions.php
	echo "" >> functions.php
	echo "function GetBinaryPrefix()" >> functions.php
	echo "{" >> functions.php
	echo '    $$WVIEW_BINARY_PREFIX' = \"$(exec_prefix)\"';' >> functions.php
	echo '    return $$WVIEW_BINARY_PREFIX;' >> functions.php
	echo "}" >> functions.php
	echo "" >> functions.php
	echo "function GetConfigPrefix()" >> functions.php
	echo "{" >> functions.php
	echo '    $$WVIEW_CONFIG_PREFIX' = \"$(sysconfdir)\"';' >> functions.php
	echo '    return $$WVIEW_CONFIG_PREFIX;' >> functions.php
	echo "}" >> functions.php
	echo "" >> functions.php
	echo "function GetDataPrefix()" >> functions.php
	echo "{" >> functions.php
	echo '    $$WVIEW_DATA_PREFIX' = \"$(localstatedir)\"';' >> functions.php
	echo '    return $$WVIEW_DATA_PREFIX;' >> functions.php
	echo "}" >> functions.php
	echo "" >> functions.php
	cat $(srcdir)/functions.php.sh >> functions.php

