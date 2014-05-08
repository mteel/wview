################################################################################
#
# File:           wviewconfig.sh
#
# Description:    Provide a script to interactively configure a wview 
#                 installation.
#
# Usage:          (must be root)
#                 wviewconfig
#                 wviewconfig get
#                 wviewconfig set [new_config_path]
#
# History:
# Engineer	  Date	    Ver   Comments
# MS Teel	  06/20/05   1    Original
# J Barber	  02/24/06   2    Partitioned into functions; added get/set
# MS Teel	  02/25/06   3    Tweaked arg and function names
# MS Teel	  06/21/08   4    Better station type support
# MS Teel	  08/23/08   5    Modify to use sqlite config database
#
################################################################################


################################################################################
#################################  M A C R O S  ################################
################################################################################
WVIEWD_PID=$WVIEW_DATA_DIR/wviewd.pid
CFG_STATION_TYPE=$STATION_TYPE

STATION_BIN=`cat $WVIEW_CONF_DIR/wview-binary`
if [ "$STATION_BIN" = "wviewd_vpro" ]; then
    STATION_TYPE="VantagePro"
else
    if [ "$STATION_BIN" = "wviewd_wxt510" ]; then
        STATION_TYPE="WXT510"
    else
        if [ "$STATION_BIN" = "wviewd_twi" ]; then
            STATION_TYPE="TWI"
        else
            if [ "$STATION_BIN" = "wviewd_ws2300" ]; then
                STATION_TYPE="WS-2300"
            else
                if [ "$STATION_BIN" = "wviewd_wmr918" ]; then
                    STATION_TYPE="WMR918"
                else
                    if [ "$STATION_BIN" = "wviewd_wmrusb" ]; then
                        STATION_TYPE="WMRUSB"
                    else
                        if [ "$STATION_BIN" = "wviewd_wh1080" ]; then
                            STATION_TYPE="WH1080"
                        else
                            if [ "$STATION_BIN" = "wviewd_te923" ]; then
                                STATION_TYPE="TE923"
                            else
                                if [ "$STATION_BIN" = "wviewd_virtual" ]; then
                                    STATION_TYPE="Virtual"
                                else
                                    STATION_TYPE="Simulator"
                                fi
                            fi
                        fi
                    fi
                fi
            fi
        fi
    fi
fi

################################################################################
#######################  D E F I N E  F U N C T I O N S  #######################
################################################################################

show_usage()
{
    echo ""
    echo "wviewconfig"
    echo "    Configures wview interactively"
    echo "wviewconfig get"
    echo "    Prints current settings in text format to stdout"
    echo "wviewconfig set [new_config_path]"
    echo "    Takes text format file of configuration at [new_config_path] and applies"
    echo "    it to the wview configuration database - it can be a partial or full list"
    echo ""
}


print_config()
{
	if [ -f $WVIEW_CONF_DIR/wview-conf.sdb ]; then
		# get settings from the sqlite database:
		echo ".separator =" > $WVIEW_CONF_DIR/commands.sql
		echo "select name,value from config;" >> $WVIEW_CONF_DIR/commands.sql
		echo ".read $WVIEW_CONF_DIR/commands.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb
		rm -rf $WVIEW_CONF_DIR/commands.sql
	else
		echo "wview Configuration database $WVIEW_CONF_DIR/wview-conf.sdb NOT FOUND - ABORTING!"
		exit 1
	fi
}


interactive_intro()
{
    echo "################################################################################"
    echo " !!!!!!!!!!!!!!!!         READ THIS BEFORE PROCEEDING         !!!!!!!!!!!!!!!!"
    echo ""
    echo "--> System Configuration for wview"
    echo ""
    echo "--> Values in parenthesis are your existing values (if they exist) or defaults - "
    echo "    they will be used if you just hit enter at the prompt..."
    echo ""
    echo "--> Note: This script will save the existing wview-conf.sdb file to" 
    echo "          $WVIEW_CONF_DIR/wview-conf.old before writing the new file" 
    echo "          based on your answers here - if that is not what you want, hit CTRL-C now to"
    echo "          abort this script!"
    echo ""
    echo "################################################################################"
    echo ""
    echo -n "pausing 3 seconds "
    sleep 1
    echo -n "."
    sleep 1
    echo -n "."
    sleep 1
    echo "."
    echo ""
}


get_wview_conf_interactive()
{
	if [ -f $WVIEW_CONF_DIR/wview-conf.sdb ]; then
		# get settings from the sqlite database:
		echo ".separator |" > $WVIEW_CONF_DIR/commands.sql
		echo "select name,value,description,dependsOn from config;" >> $WVIEW_CONF_DIR/commands.sql
		echo ".read $WVIEW_CONF_DIR/commands.sql" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > $WVIEW_CONF_DIR/parms.out
		rm -rf $WVIEW_CONF_DIR/commands.sql
	
		# Construct the editor script:
		echo "#!/bin/sh"											> $WVIEW_CONF_DIR/editparm
		echo "if [ \"\" != \"\$4\" ]; then"									>> $WVIEW_CONF_DIR/editparm
		echo "    echo \"select value from config where name='\$4';\" > $WVIEW_CONF_DIR/cmnd.sql"		>> $WVIEW_CONF_DIR/editparm
		echo "    echo \".read $WVIEW_CONF_DIR/cmnd.sql\" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb > $WVIEW_CONF_DIR/value.out"	>> $WVIEW_CONF_DIR/editparm
		echo "    IS_ENABLED=\`cat $WVIEW_CONF_DIR/value.out\`"							>> $WVIEW_CONF_DIR/editparm
		echo "    rm -rf $WVIEW_CONF_DIR/cmnd.sql $WVIEW_CONF_DIR/value.out"					>> $WVIEW_CONF_DIR/editparm
		echo "else"												>> $WVIEW_CONF_DIR/editparm
		echo "    IS_ENABLED=yes"										>> $WVIEW_CONF_DIR/editparm
		echo "fi"												>> $WVIEW_CONF_DIR/editparm
		echo "if [ \"\$IS_ENABLED\" != \"yes\" ]; then"								>> $WVIEW_CONF_DIR/editparm
		echo "    exit 0"											>> $WVIEW_CONF_DIR/editparm
		echo "fi"												>> $WVIEW_CONF_DIR/editparm
		echo "NEWVAL=\$2"											>> $WVIEW_CONF_DIR/editparm
		echo "echo \"-------------------------------------------------------------\""				>> $WVIEW_CONF_DIR/editparm
		echo "echo \"\$3\""											>> $WVIEW_CONF_DIR/editparm
		echo "echo \"PARAMETER: \$1\""										>> $WVIEW_CONF_DIR/editparm
		echo "echo -n \"(\$2): \""										>> $WVIEW_CONF_DIR/editparm
		echo "read INVAL"											>> $WVIEW_CONF_DIR/editparm
		echo "if [ \"\" != \"\$INVAL\" ]; then"									>> $WVIEW_CONF_DIR/editparm
		echo "    echo \"update config set value='\$INVAL' where name='\$1';\" > $WVIEW_CONF_DIR/cmnd.sql"	>> $WVIEW_CONF_DIR/editparm
		echo "    echo \".read $WVIEW_CONF_DIR/cmnd.sql\" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb"		>> $WVIEW_CONF_DIR/editparm
		echo "    rm -rf $WVIEW_CONF_DIR/cmnd.sql"								>> $WVIEW_CONF_DIR/editparm
		echo "    if [ \"\$1\" = \"STATION_TYPE\" ]; then"								>> $WVIEW_CONF_DIR/editparm
		echo "        if [ \"\$INVAL\" = \"VantagePro\" ]; then"						>> $WVIEW_CONF_DIR/editparm
		echo "            echo \"wviewd_vpro\" > $WVIEW_CONF_DIR/wview-binary"			>> $WVIEW_CONF_DIR/editparm
		echo "        else"			>> $WVIEW_CONF_DIR/editparm
		echo "            if [ \"\$INVAL\" = \"WXT510\" ]; then"						>> $WVIEW_CONF_DIR/editparm
		echo "                echo \"wviewd_wxt510\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "            else"			>> $WVIEW_CONF_DIR/editparm
		echo "                if [ \"\$INVAL\" = \"WS-2300\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                    echo \"wviewd_ws2300\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                else"			>> $WVIEW_CONF_DIR/editparm
		echo "                    if [ \"\$INVAL\" = \"WMR918\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                        echo \"wviewd_wmr918\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                    else"			>> $WVIEW_CONF_DIR/editparm
		echo "                        if [ \"\$INVAL\" = \"WMRUSB\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                            echo \"wviewd_wmrusb\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                        else"			>> $WVIEW_CONF_DIR/editparm
		echo "                            if [ \"\$INVAL\" = \"WH1080\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                                echo \"wviewd_wh1080\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                            else"			>> $WVIEW_CONF_DIR/editparm
		echo "                                if [ \"\$INVAL\" = \"Simulator\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                                    echo \"wviewd_sim\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                                else"			>> $WVIEW_CONF_DIR/editparm
		echo "                                    if [ \"\$INVAL\" = \"TWI\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                                        echo \"wviewd_twi\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                                    else"			>> $WVIEW_CONF_DIR/editparm
		echo "                                        if [ \"\$INVAL\" = \"TE923\" ]; then"					>> $WVIEW_CONF_DIR/editparm
		echo "                                            echo \"wviewd_te923\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                                        else"			>> $WVIEW_CONF_DIR/editparm
		echo "                                            if [ \"\$INVAL\" = \"Virtual\" ]; then"				>> $WVIEW_CONF_DIR/editparm
		echo "                                                echo \"wviewd_virtual\" > $WVIEW_CONF_DIR/wview-binary"	>> $WVIEW_CONF_DIR/editparm
		echo "                                            fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                                        fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                                    fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                                fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                            fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                        fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                    fi"												>> $WVIEW_CONF_DIR/editparm
		echo "                fi"												>> $WVIEW_CONF_DIR/editparm
		echo "            fi"												>> $WVIEW_CONF_DIR/editparm
		echo "        fi"												>> $WVIEW_CONF_DIR/editparm
		echo "    fi"												>> $WVIEW_CONF_DIR/editparm
		echo "fi"												>> $WVIEW_CONF_DIR/editparm
		chmod +x $WVIEW_CONF_DIR/editparm

        cd $WVIEW_CONF_DIR

		# Edit parms one at a time:
		gawk -F"|" '{ 
		sysstring=sprintf("./editparm \"%s\" \"%s\" \"%s\" \"%s\"", $1, $2, $3, $4)
		system(sysstring)
		}' $WVIEW_CONF_DIR/parms.out
	
		rm -rf $WVIEW_CONF_DIR/editparm $WVIEW_CONF_DIR/parms.out
	else
		echo "wview Configuration database $WVIEW_CONF_DIR/wview-conf.sdb NOT FOUND - ABORTING!"
		exit 1
	fi
}


set_config_from_file()
{
	if [ -f $WVIEW_CONF_DIR/wview-conf.sdb ]; then
		# Construct the update script:
		echo "#!/bin/sh"											> $WVIEW_CONF_DIR/updateparm
		echo "if [ \"\" != \"\$2\" ]; then"									>> $WVIEW_CONF_DIR/updateparm
		echo "    echo \"update config set value='\$2' where name='\$1';\" > $WVIEW_CONF_DIR/cmnd.sql"		>> $WVIEW_CONF_DIR/updateparm
		echo "    echo \".read $WVIEW_CONF_DIR/cmnd.sql\" | sqlite3 $WVIEW_CONF_DIR/wview-conf.sdb"		>> $WVIEW_CONF_DIR/updateparm
		echo "    rm -rf $WVIEW_CONF_DIR/cmnd.sql"								>> $WVIEW_CONF_DIR/updateparm
		echo "fi"												>> $WVIEW_CONF_DIR/updateparm
		chmod +x $WVIEW_CONF_DIR/updateparm

        cd $WVIEW_CONF_DIR

		# Update parms one at a time:
		gawk -F"=" '{ 
		sysstring=sprintf("./updateparm \"%s\" \"%s\"", $1, $2)
		system(sysstring)
		}' $SET_FILE
	
		rm -rf $WVIEW_CONF_DIR/updateparm
	else
		echo "wview Configuration database $WVIEW_CONF_DIR/wview-conf.sdb NOT FOUND - ABORTING!"
		exit 1
	fi	
}


################################################################################
##################  S C R I P T  E X E C U T I O N  S T A R T  #################
################################################################################

# First test to make sure that wview is not running for interactive and set...
if [ -f $WVIEWD_PID -a "$1" != "get" ]; then
    echo "wviewd is running - stop wview before running this script..."
    exit 3
fi

# Make sure that the config DB is there:
if [ ! -f $WVIEW_CONF_DIR/wview-conf.sdb ]; then
    echo "wview configuration database NOT FOUND"
    exit 4
fi


METHOD=$1
SET_FILE=$2

if [ "$METHOD" = "" ]   # run interactively
then
    interactive_intro
    get_wview_conf_interactive

echo ""
echo ""
echo "################################################################################"
echo "--> wview Configuration Complete!"
echo "--> Now run wviewhtmlconfig to select your site skin."
echo "################################################################################"
else
    case "$METHOD" in
        "get" )
            print_config
            ;;
        "set" )
            if [ "$SET_FILE" = "" ]; then
                echo "set requires a source file:"
                show_usage
                exit 1
            fi
            if [ ! -f $SET_FILE ]; then
                echo "source path $SET_FILE does not exist"
                show_usage
                exit 1
            fi

            set_config_from_file
            ;;
        *)
            echo "$METHOD not supported"
            show_usage
            exit 1
            ;;
    esac
fi

exit 0

