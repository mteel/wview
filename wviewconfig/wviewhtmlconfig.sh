################################################################################
#
# File:           wviewhtmlconfig.sh
#
# Description:    Provide a script to interactively configure wview HTML 
#                 templates.
#
# Usage:          (must be root)
#                 wviewhtmlconfig <skin_selection>
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel         02/05/08   1    Original
# MS Teel         09/13/08   2    Modify for SQLite configuration DB
# MS Teel         05/03/09   3    Add non-interactive support
# MS Teel         03/12/12   4    Add exfoliation skin.
#
################################################################################


################################################################################
#################################  M A C R O S  ################################
################################################################################


################################################################################
#######################  D E F I N E  F U N C T I O N S  #######################
################################################################################

get_wviewconfig()
{
    echo "select value from config where name='HTMLGEN_EXTENDED_DATA';" > $WVIEW_CONF_DIR/cmnd.sql
    echo ".read $WVIEW_CONF_DIR/cmnd.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > $WVIEW_CONF_DIR/value.out
    DATA_EXTENDED=`cat $WVIEW_CONF_DIR/value.out`

    echo "select value from config where name='HTMLGEN_METRIC_UNITS';" > $WVIEW_CONF_DIR/cmnd.sql
    echo ".read $WVIEW_CONF_DIR/cmnd.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > $WVIEW_CONF_DIR/value.out
    STATION_METRIC=`cat $WVIEW_CONF_DIR/value.out`

    echo "select value from config where name='HTMLGEN_IMAGE_PATH';" > $WVIEW_CONF_DIR/cmnd.sql
    echo ".read $WVIEW_CONF_DIR/cmnd.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > $WVIEW_CONF_DIR/value.out
    HTML_DESTINATION=`cat $WVIEW_CONF_DIR/value.out`

    rm -rf $WVIEW_CONF_DIR/cmnd.sql
    rm -rf $WVIEW_CONF_DIR/value.out
}

show_skins()
{
    echo ""
    echo "wview site skins currently supported:"
    echo "    classic - default wview skin"
    echo "    chrome  - classic with a chrome effect"
    echo "    exfoliation - a clean implementation by Matthew Wall"
}

interactive_intro()
{
    echo "################################################################################"
    echo " !!!!!!!!!!!!!!!!         READ THIS BEFORE PROCEEDING         !!!!!!!!!!!!!!!!"
    echo ""
    echo "--> HTML Template Configuration for wview"
    echo ""
    echo "--> Note: This script will save existing templates to $WVIEW_CONF_DIR/html-YYYYMMDD.HHmmSS"
    echo "          before writing the new files based on your answers here - if that"
    echo "          is not what you want, hit CTRL-C now to abort this script!"
    echo ""
    echo "################################################################################"
    echo ""
    echo -n "pausing 5 seconds "
    sleep 1
    echo -n "."
    sleep 1
    echo -n "."
    sleep 1
    echo -n "."
    sleep 1
    echo -n "."
    sleep 1
    echo "."
    echo ""
}

save_old()
{
    OLD_HTML_SAVE=`date '+%Y%m%d.%H%M%S'`
    echo "Saving old HTML directory to html-$OLD_HTML_SAVE ..."
    cp -rf $WVIEW_CONF_DIR/html $WVIEW_CONF_DIR/html-$OLD_HTML_SAVE
    echo "...done."
    echo "Saving old config files..."
    if [ -f $WVIEW_CONF_DIR/html-templates.conf ]; then
        mv $WVIEW_CONF_DIR/html-templates.conf $WVIEW_CONF_DIR/html-templates.conf-$OLD_HTML_SAVE
    fi
    if [ -f $WVIEW_CONF_DIR/images.conf ]; then
        mv $WVIEW_CONF_DIR/images.conf $WVIEW_CONF_DIR/images.conf-$OLD_HTML_SAVE
    fi
    if [ -f $WVIEW_CONF_DIR/images-user.conf ]; then
        mv $WVIEW_CONF_DIR/images-user.conf $WVIEW_CONF_DIR/images-user.conf-$OLD_HTML_SAVE
    fi
    if [ -f $WVIEW_CONF_DIR/graphics.conf ]; then
        mv $WVIEW_CONF_DIR/graphics.conf $WVIEW_CONF_DIR/graphics.conf-$OLD_HTML_SAVE
    fi
    if [ -f $WVIEW_CONF_DIR/pre-generate.sh ]; then
        mv $WVIEW_CONF_DIR/pre-generate.sh $WVIEW_CONF_DIR/pre-generate.sh-$OLD_HTML_SAVE
    fi
    if [ -f $WVIEW_CONF_DIR/post-generate.sh ]; then
        mv $WVIEW_CONF_DIR/post-generate.sh $WVIEW_CONF_DIR/post-generate.sh-$OLD_HTML_SAVE
    fi
    echo "... done."
}

remove_all()
{
	rm -f $WVIEW_CONF_DIR/html/*.*
	rm -f $WVIEW_CONF_DIR/html-templates.conf
	rm -f $WVIEW_CONF_DIR/images.conf
	rm -f $WVIEW_CONF_DIR/images-user.conf
	rm -f $WVIEW_CONF_DIR/graphics.conf
	rm -f $WVIEW_CONF_DIR/pre-generate.sh
	rm -f $WVIEW_CONF_DIR/post-generate.sh    
}

get_user_preferences()
{
    echo ""
    echo "---------------------------------------------------------------------------" 
    echo "Which template skin do you want to use for your site?"
    show_skins
    echo -n "($SITE_SKIN): "
    read INVAL
    if [ "" != "$INVAL" ]; then
        SITE_SKIN=$INVAL
    fi
    case "$SITE_SKIN" in
        "classic" )
            echo "Site skin $SITE_SKIN selected..."
            ;;
        "chrome" )
            echo "Site skin $SITE_SKIN selected..."
            ;;
        "exfoliation" )
            echo "Site skin $SITE_SKIN selected..."
            ;;
        *)
            echo "$SITE_SKIN not supported!"
            show_skins
            echo "Aborting - run wviewhtmlconfig again and select a valid skin..."
            exit 1
            ;;
    esac

    if [ "yes" = "$DATA_EXTENDED" ]; then
        echo "With extended data graphics."
    else
        echo "With NO extended data graphics."
    fi
    if [ "yes" = "$STATION_METRIC" ]; then
        echo "With metric units."
    else
        echo "With US/Imperial units."
    fi
}

move_templates()
{
    if [ "yes" = "$DATA_EXTENDED" ]; then
        cp -rf $WVIEW_CONF_DIR/html/$SITE_SKIN/plus/*.* $WVIEW_CONF_DIR/html
    else
        cp -rf $WVIEW_CONF_DIR/html/$SITE_SKIN/standard/*.* $WVIEW_CONF_DIR/html
    fi

    if [ -d $WVIEW_CONF_DIR/html/$SITE_SKIN/static ]; then
        cp -rf $WVIEW_CONF_DIR/html/$SITE_SKIN/static/*.* $HTML_DESTINATION
    fi
}

setup_conf_files()
{
    cp -f $WVIEW_CONF_DIR/html/html-templates.conf $WVIEW_CONF_DIR
    cp -f $WVIEW_CONF_DIR/html/pre-generate.sh $WVIEW_CONF_DIR
	chmod +x $WVIEW_CONF_DIR/pre-generate.sh
    cp -f $WVIEW_CONF_DIR/html/post-generate.sh $WVIEW_CONF_DIR
	chmod +x $WVIEW_CONF_DIR/post-generate.sh
    cp -f $WVIEW_CONF_DIR/html/images-user.conf $WVIEW_CONF_DIR
    cp -f $WVIEW_CONF_DIR/html/graphics.conf $WVIEW_CONF_DIR
    if [ "yes" = "$STATION_METRIC" ]; then
        cp -f $WVIEW_CONF_DIR/html/images-metric.conf $WVIEW_CONF_DIR/images.conf
        cp -f $WVIEW_CONF_DIR/html/awekas_wl.htx-metric $WVIEW_CONF_DIR/html/awekas_wl.htx
    else
        cp -f $WVIEW_CONF_DIR/html/images.conf $WVIEW_CONF_DIR/images.conf
        cp -f $WVIEW_CONF_DIR/html/awekas_wl.htx-us $WVIEW_CONF_DIR/html/awekas_wl.htx
    fi
}

################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################
export PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin

get_wviewconfig

#  Is there an argument?
if [ "" != "$1" ]; then
    if [ "$1" = "classic" ]; then
        SITE_SKIN=$1
        INTERACTIVE=0
    elif [ "$1" = "chrome" ]; then
        SITE_SKIN=$1
        INTERACTIVE=0
    elif [ "$1" = "exfoliation" ]; then
        SITE_SKIN=$1
        INTERACTIVE=0
    elif [ "$1" = "remove" ]; then
        REMOVE_IT=1
    else
        SITE_SKIN="chrome"
        INTERACTIVE=1
    fi
else
    REMOVE_IT=0
    SITE_SKIN="chrome"
    INTERACTIVE=1
fi

if [ "$REMOVE_IT" = "1" ]; then
    remove_all
    echo "HTML setup files removed"
    exit 0
fi

if [ "$INTERACTIVE" = "1" ]; then
    interactive_intro
    get_user_preferences
    save_old
fi

move_templates
setup_conf_files
echo "HTML setup complete:"
echo "you may now customize template files in $WVIEW_CONF_DIR/html"

