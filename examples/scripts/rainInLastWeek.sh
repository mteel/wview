#!/bin/sh

## rainInLastWeek.sh
##
## Script to retrieve the last week's rain.
## Change the value 604800 (number of seconds in a week) if you want a different
## time frame.
##
## Contributed by n1otx

## Define the location of the archive database (adjust if necessary):
#WVIEW_ARCHIVE_DB=/usr/local/var/wview/archive/wview-archive.sdb
#WVIEW_ARCHIVE_DB=/opt/local/var/wview/archive/wview-archive.sdb
WVIEW_ARCHIVE_DB=/var/lib/wview/archive/wview-archive.sdb

sqlite3 $WVIEW_ARCHIVE_DB \
    "SELECT sum(rain) As Sum FROM archive WHERE dateTime BETWEEN \
    strftime('%s','now')-604800 AND strftime('%s','now');"
