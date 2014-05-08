exfoliation for wview
Copyright 2011 Matthew Wall, all rights reserved

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

This is a skin for Mark Teel's wview weather collection/display software.
It has been tested with wview 5.19.0, both standard and extended data.

This skin was created to provide a baseline display of weather data in a
compact, fairly minimalist format.  The URLs for the web pages generated
by this skin are backward-compatible to at least wview 5.19.0.  The intent
is to remain close to the original classic and chrome skins in terms of
information content, but clean up the layout and implementation.

This skin also adds a 'phone' page targeted toward mobile devices.  Thanks
to MarkF for the initial phone implementation.

Revision History
  0.7 12feb12
   * added the plus configuration
   * adjustments to layout of phone page
  0.6 12dec11
   * added post-generate.sh script to create NOAA and Archive directories
       and copy static elements to the web directory if they do not already
       exist there.  this is primarily useful when the img directory is a
       ramdisk, since its contents will be obliterated on system restarts.
  0.5 21nov11
   * forgot to update html-templates.conf and graphics.conf
  0.4 21nov11
   * use skin name for the css and js files to avoid conflicts
   * use width/height on image classes to speed rendering
   * simplify the navigation controls
   * keep the reporting forms on the almanac page only to reduce clutter
   * added phone.htx for mobile phone display
  0.3 19nov11
   * remove more extraneous files
   * subdue the date/time for hi/ho readings in almanac
   * simplify Current_Conditions even more
   * added a simple makefile to automate the housekeeping chores
   * remove more extraneous markup and spaces from readings tables
   * apply date/time css to readings table
  0.2 19nov11
   * fix a few typos in the readings tables
   * share the hi/lo readings across all time periods
   * use bold for the metric values
   * added time/date of the hi/lo readings to the hi/lo table
   * reorder the graphs to be consistent across all pages
   * reorder items in the hi/lo table to follow graph order
   * remove everything (almost) from the current page that is not current
   * tested on ie6, ff3, safari5, opera11
  0.1 16nov11
   * initial release


Installation Instructions

To install the skin, do something like this:

# stop the wview daemons
/etc/init.d/wview stop

# make a backup of the current configuration and templates
cd /etc/wview
tar cvfz site-template-yymmdd.tgz html *.conf *.sh
# make a backup of the current site
cd /var/wview
tar cvfz site-yymmdd.tgz img

# extract the exfoliation contents and install them
cd /etc/wview/skins
tar xvfz exfoliation-for-wview-x.y.z.tgz
cp exfoliation/static/* /var/wview/img
cp exfoliation/standard/*.htx /etc/wview/html
cp exfoliation/standard/*.incx /etc/wview/html
cp exfoliation/standard/*.conf /etc/wview

# start the wview daemons
/etc/init.d/wview start
