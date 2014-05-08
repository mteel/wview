#/bin/sh

# configure the img directory if it is not already configured.  this is for
# flash-based systems where the img directory is a ramdisk.
STATIC_DIR=/opt/wview/etc/skins/exfoliation/static
WWW_DIR=/var/wview/img
for d in NOAA Archive; do
    if [ ! -d $WWW_DIR/$d ]; then
        mkdir $WWW_DIR/$d
    fi
done
for f in exfoliation.css exfoliation.js; do
    if [ ! -f $WWW_DIR/$f ]; then
        cp $SKIN_DIR/$f $WWW_DIR
    fi
done

exit 0

