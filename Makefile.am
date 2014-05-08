# Makefile - top level

# files to include in the distro but not build
EXTRA_DIST = reconf \
             $(top_srcdir)/stations/VantagePro/doc \
             $(top_srcdir)/bin \
             $(top_srcdir)/examples/alarms \
             $(top_srcdir)/examples/conf \
             $(top_srcdir)/examples/html \
             $(top_srcdir)/examples/scripts \
             $(top_srcdir)/examples/rsyslog \
             $(top_srcdir)/debian \
             $(top_srcdir)/scripts \
             $(top_srcdir)/cross-compile \
             $(top_srcdir)/alarms/sample-datafeed-client \
             $(top_srcdir)/ftp/wviewftp.debug.sh \
             $(top_srcdir)/UPGRADE \
             $(top_srcdir)/MAINTAIN \
             $(top_srcdir)/wview-User-Manual.html \
             $(top_srcdir)/wview-Quick-Start.html \
             $(top_srcdir)/wview-Debian-Quick-Start.html \
             $(top_srcdir)/wview-Quick-Start-MacOSX.html \
             $(top_srcdir)/wview-Quick-Start-FreeBSD.html

SUBDIRS = stations htmlgenerator alarms cwop http ftp ssh procmon wviewconfig wviewmgmt utilities dbexport examples

STATION_T = wviewd_sim

WV_CONFIG_DIR		= $(sysconfdir)/wview
WV_RUN_DIR			= $(localstatedir)/wview
WV_HTML_DIR			= $(localstatedir)
WV_BIN_DIR			= $(exec_prefix)/bin
WV_SHARE_DIR		= /usr/share/wview


# this target removes svn directories from the tarball
dist-hook:
	rm -rf `find $(distdir) -name .svn`


# Keep this for backwards compatibility:
install-env:	install


# Do some post-install tasks:
install-data-hook:
if INSTALL_DPKG
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/noaa
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/alarms
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/img
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/img/Archive
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/img/NOAA
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview/archive
	cp -fp $(top_srcdir)/bin/img/*.* $(DESTDIR)$(WV_SHARE_DIR)/var/wview/img
	cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_SHARE_DIR)/var/wview/img
	cp -fp $(top_srcdir)/bin/archive/* $(DESTDIR)$(WV_SHARE_DIR)/var/wview/archive
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/export
	chmod 777 $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/export
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/plus
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/static
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/standard
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/plus
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/static
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/standard
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/plus
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/static
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/standard
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/alarms
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/rsyslog.d
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/logrotate.d
	cp -fp $(top_srcdir)/debian/99-wview.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/rsyslog.d
	cp -fp $(top_srcdir)/debian/wview-logrotate $(DESTDIR)$(WV_SHARE_DIR)/etc/logrotate.d
	@echo "$(PACKAGE_VERSION)" > $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/wview-version
	cp -fp $(top_srcdir)/examples/html/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html
	cp -fp $(top_srcdir)/examples/html/chrome/plus/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/plus
	cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/static
	cp -fp $(top_srcdir)/examples/html/chrome/standard/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/chrome/standard
	cp -fp $(top_srcdir)/examples/html/classic/plus/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/plus
	cp -fp $(top_srcdir)/examples/html/classic/static/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/static
	cp -fp $(top_srcdir)/examples/html/classic/standard/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/classic/standard
	cp -fp $(top_srcdir)/examples/html/exfoliation/plus/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/plus
	cp -fp $(top_srcdir)/examples/html/exfoliation/static/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/static
	cp -fp $(top_srcdir)/examples/html/exfoliation/standard/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/exfoliation/standard
	cp -fp $(top_srcdir)/examples/alarms/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/alarms
	cp -fp $(top_srcdir)/examples/conf/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/graphics.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/html-templates.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/pre-generate.sh $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/post-generate.sh $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/images-user.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/images.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	cp -fp $(top_srcdir)/examples/html/chrome/standard/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html
	cp -fp $(top_srcdir)/examples/html/chrome/standard/awekas_wl.htx-us $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html/awekas_wl.htx
	cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/html
else
	@if [ ! -d $(DESTDIR)$(WV_RUN_DIR) ]; then \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR); \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/noaa; \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/alarms; \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/img; \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/img/Archive; \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/img/NOAA; \
		mkdir -p $(DESTDIR)$(WV_RUN_DIR)/archive; \
		cp -fp $(top_srcdir)/bin/img/*.* $(DESTDIR)$(WV_RUN_DIR)/img; \
		cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_RUN_DIR)/img; \
		cp -fp $(top_srcdir)/bin/archive/* $(DESTDIR)$(WV_RUN_DIR)/archive; \
		sqlite3 $(DESTDIR)$(WV_RUN_DIR)/archive/wview-archive.sdb '.read $(DESTDIR)$(WV_RUN_DIR)/archive/wview-archive.sql'; \
		echo "==> $(DESTDIR)$(WV_RUN_DIR) has been created with distro examples"; \
	else \
		echo "==> $(DESTDIR)$(WV_RUN_DIR) already exists - skipping default data installation"; \
	fi
	@if [ ! -e $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb ]; then \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR); \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/export; \
		chmod 777 $(DESTDIR)$(WV_CONFIG_DIR)/export; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/plus; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/static; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/standard; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/classic; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/plus; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/static; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/standard; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/plus; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/static; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/standard; \
		mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)/alarms; \
		cp -fp $(top_srcdir)/examples/html/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html; \
		cp -fp $(top_srcdir)/examples/html/chrome/plus/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/plus; \
		cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/static; \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/chrome/standard; \
		cp -fp $(top_srcdir)/examples/html/classic/plus/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/plus; \
		cp -fp $(top_srcdir)/examples/html/classic/static/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/static; \
		cp -fp $(top_srcdir)/examples/html/classic/standard/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/classic/standard; \
		cp -fp $(top_srcdir)/examples/html/exfoliation/plus/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/plus; \
		cp -fp $(top_srcdir)/examples/html/exfoliation/static/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/static; \
		cp -fp $(top_srcdir)/examples/html/exfoliation/standard/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html/exfoliation/standard; \
		cp -fp $(top_srcdir)/examples/alarms/*.* $(DESTDIR)$(WV_CONFIG_DIR)/alarms; \
		cp -fp $(top_srcdir)/examples/conf/*.* $(DESTDIR)$(WV_CONFIG_DIR); \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/graphics.conf $(DESTDIR)$(WV_CONFIG_DIR); \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/html-templates.conf $(DESTDIR)$(WV_CONFIG_DIR); \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/pre-generate.sh $(DESTDIR)$(WV_CONFIG_DIR); \
		chmod +x $(DESTDIR)$(WV_CONFIG_DIR)/pre-generate.sh; \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/post-generate.sh $(DESTDIR)$(WV_CONFIG_DIR); \
		chmod +x $(DESTDIR)$(WV_CONFIG_DIR)/post-generate.sh; \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/images-user.conf $(DESTDIR)$(WV_CONFIG_DIR); \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/images.conf $(DESTDIR)$(WV_CONFIG_DIR); \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html; \
		cp -fp $(top_srcdir)/examples/html/chrome/standard/awekas_wl.htx-us $(DESTDIR)$(WV_CONFIG_DIR)/html/awekas_wl.htx; \
		cp -fp $(top_srcdir)/examples/html/chrome/static/*.* $(DESTDIR)$(WV_CONFIG_DIR)/html; \
		sqlite3 $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb '.read $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sql'; \
		echo "==> $(DESTDIR)$(WV_CONFIG_DIR) has been created with distro examples"; \
		echo ""; \
		echo "This is a first time install so the station type will be set to \"simulator\"."; \
		echo "To change the station type, run \"wviewconfig\" or use the wviewmgmt web interface to change it."; \
		echo "You will need to stop wview and clear the databases of simulator data after you have changed the station type:"; \
		echo "#> wviewcleardata"; \
		echo ""; \
		echo "This is a first time install so the generation templates will be set to \"chrome-standard-US units\"."; \
		echo "To change this run \"wviewhtmlconfig\" to configure the template directory"; \
		echo "after you have configured wview for metric units and/or extended data."; \
		echo ""; \
		echo "If you want start/stop control of wview from the Management Web Site"; \
		echo "(and you are comfortable giving the http server user account sudo privileges):"; \
		echo "Add the http user to the sudo group:"; \
		echo "#> sudo adduser www-data sudo"; \
		echo "Set the sudo group to no password required privileges in /etc/sudoers:"; \
		echo "#> sudo visudo"; \
		echo "(Make sure the line [%sudo ALL=NOPASSWD: ALL] is at the bottom of the /etc/sudoers file)"; \
		echo ""; \
	else \
		echo "==> $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb already exists - skipping default configuration installation"; \
		cp -fp $(top_srcdir)/examples/conf/wview-conf-update.sql $(DESTDIR)$(WV_CONFIG_DIR); \
		sqlite3 $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb '.read $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf-update.sql'; \
	fi
	@echo "To start/stop wview:"
	@echo "#> sudo /etc/init.d/wview start|stop"
	@echo "$(PACKAGE_VERSION)" > $(DESTDIR)$(WV_CONFIG_DIR)/wview-version
endif
	@mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)
	@chmod 777 $(DESTDIR)$(WV_CONFIG_DIR)
	@if [ ! -e $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary ]; then \
		echo "$(STATION_T)" > $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary; \
	fi
	@chmod 666 $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary
	@if [ -e $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb ]; then \
		chmod 666 $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb; \
	fi
	@mkdir -p $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt
	@mkdir -p $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt/imgs
	@cp -fp $(top_srcdir)/wviewmgmt/*.* $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt
	@cp -fp $(top_srcdir)/wviewmgmt/imgs/*.* $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt/imgs
	@if [ ! -e $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt/functions.php ]; then \
		$(LN_S) $(DESTDIR)$(WV_BIN_DIR)/functions.php $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt/functions.php; \
	fi

uninstall-hook:
	rm -rf $(DESTDIR)$(WV_RUN_DIR)
	rm -rf $(DESTDIR)$(WV_HTML_DIR)/wviewmgmt
	rm -rf $(DESTDIR)$(WV_CONFIG_DIR)
	@if [ -e $(DESTDIR)$(sysconfdir)/init.d/wview ]; then \
		rm -f $(DESTDIR)$(sysconfdir)/init.d/wview; \
	fi

