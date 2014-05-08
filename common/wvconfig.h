#ifndef INC_wvconfigh
#define INC_wvconfigh
/*---------------------------------------------------------------------------
 
  FILENAME:
        wvconfig.h
 
  PURPOSE:
        Define the wview configuration API.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        07/05/2008      M.S. Teel       0               Original
 
  NOTES:
        
 
  LICENSE:
        Copyright (c) 2008, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

#include <radshmem.h>
#include <radlist.h>
#include <radbuffers.h>
#include <radsemaphores.h>
#include <radsqlite.h>

#include <sysdefs.h>


//  ... macro definitions


//  ... typedefs

// wview configuration item IDs:

#define configItem_ENABLE_HTMLGEN                               "ENABLE_HTMLGEN"
#define configItem_ENABLE_ALARMS                                "ENABLE_ALARMS"
#define configItem_ENABLE_CWOP                                  "ENABLE_CWOP"
#define configItem_ENABLE_HTTP                                  "ENABLE_HTTP"
#define configItem_ENABLE_FTP                                   "ENABLE_FTP"
#define configItem_ENABLE_SSH                                   "ENABLE_SSH"
#define configItem_ENABLE_PROCMON                               "ENABLE_PROCMON"
#define configItem_ENABLE_SQL                                   "ENABLE_SQL"

#define configItem_ENABLE_EMAIL                                 "ENABLE_EMAIL_ALERTS"
#define configItem_TO_EMAIL_ADDRESS                             "EMAIL_ADDRESS"
#define configItem_FROM_EMAIL_ADDRESS                           "FROM_EMAIL_ADDRESS"
#define configItem_SEND_TEST_EMAIL                              "SEND_TEST_EMAIL"

#define configItem_STATION_STATION_TYPE                         "STATION_TYPE"
#define configItem_STATION_STATION_INTERFACE                    "STATION_INTERFACE"
#define configItem_STATION_STATION_DEV                          "STATION_DEV"
#define configItem_STATION_STATION_HOST                         "STATION_HOST" 
#define configItem_STATION_STATION_PORT                         "STATION_PORT"
#define configItem_STATION_STATION_WLIP                         "STATION_WLIP"
#define configItem_STATION_STATION_RETRIEVE_ARCHIVE             "STATION_RETRIEVE_ARCHIVE"
#define configItem_STATION_STATION_DTR                          "STATION_DTR"
#define configItem_STATION_STATION_RAIN_SEASON_START            "STATION_RAIN_SEASON_START"
#define configItem_STATION_STATION_RAIN_STORM_TRIGGER_START     "STATION_RAIN_STORM_TRIGGER_START"
#define configItem_STATION_STATION_RAIN_STORM_IDLE_STOP         "STATION_RAIN_STORM_IDLE_STOP"
#define configItem_STATION_STATION_RAIN_YTD                     "STATION_RAIN_YTD"
#define configItem_STATION_STATION_ET_YTD                       "STATION_ET_YTD" 
#define configItem_STATION_STATION_RAIN_ET_YTD_YEAR             "STATION_RAIN_ET_YTD_YEAR"
#define configItem_STATION_STATION_ELEVATION                    "STATION_ELEVATION"
#define configItem_STATION_STATION_LATITUDE                     "STATION_LATITUDE"
#define configItem_STATION_STATION_LONGITUDE                    "STATION_LONGITUDE"
#define configItem_STATION_STATION_ARCHIVE_INTERVAL             "STATION_ARCHIVE_INTERVAL"
#define configItem_STATION_ARCHIVE_PATH                         "STATION_ARCHIVE_PATH"
#define configItem_STATION_POLL_INTERVAL                        "STATION_POLL_INTERVAL"
#define configItem_STATION_PUSH_INTERVAL                        "STATION_PUSH_INTERVAL"
#define configItem_STATION_VERBOSE_MSGS                         "STATION_VERBOSE_MSGS"
#define configItem_STATION_DO_RXCHECK                           "STATION_DO_RCHECK"
#define configItem_STATION_OUTSIDE_CHANNEL                      "STATION_OUTSIDE_CHANNEL"

#define configItem_HTMLGEN_STATION_NAME                         "HTMLGEN_STATION_NAME" 
#define configItem_HTMLGEN_STATION_CITY                         "HTMLGEN_STATION_CITY" 
#define configItem_HTMLGEN_STATION_STATE                        "HTMLGEN_STATION_STATE" 
#define configItem_HTMLGEN_STATION_SHOW_IF                      "HTMLGEN_STATION_SHOW_IF" 
#define configItem_HTMLGEN_IMAGE_PATH                           "HTMLGEN_IMAGE_PATH" 
#define configItem_HTMLGEN_HTML_PATH                            "HTMLGEN_HTML_PATH" 
#define configItem_HTMLGEN_START_OFFSET                         "HTMLGEN_START_OFFSET" 
#define configItem_HTMLGEN_GENERATE_INTERVAL                    "HTMLGEN_GENERATE_INTERVAL" 
#define configItem_HTMLGEN_METRIC_UNITS                         "HTMLGEN_METRIC_UNITS" 
#define configItem_HTMLGEN_METRIC_USE_RAIN_MM                   "HTMLGEN_METRIC_USE_RAIN_MM"
#define configItem_HTMLGEN_WIND_UNITS                           "HTMLGEN_WIND_UNITS"
#define configItem_HTMLGEN_DUAL_UNITS                           "HTMLGEN_DUAL_UNITS" 
#define configItem_HTMLGEN_EXTENDED_DATA                        "HTMLGEN_EXTENDED_DATA" 
#define configItem_HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP        "HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP" 
#define configItem_HTMLGEN_MPHASE_INCREASE                      "HTMLGEN_MPHASE_INCREASE" 
#define configItem_HTMLGEN_MPHASE_DECREASE                      "HTMLGEN_MPHASE_DECREASE" 
#define configItem_HTMLGEN_MPHASE_FULL                          "HTMLGEN_MPHASE_FULL" 
#define configItem_HTMLGEN_LOCAL_RADAR_URL                      "HTMLGEN_LOCAL_RADAR_URL" 
#define configItem_HTMLGEN_LOCAL_FORECAST_URL                   "HTMLGEN_LOCAL_FORECAST_URL" 
#define configItem_HTMLGEN_DATE_FORMAT                          "HTMLGEN_DATE_FORMAT"  
 
#define configItem_ALARMS_STATION_METRIC                        "ALARMS_STATION_METRIC"
#define configItem_ALARMS_DO_TEST                               "ALARMS_DO_TEST"
#define configItem_ALARMS_DO_TEST_NUMBER                        "ALARMS_DO_TEST_NUMBER"
#define configItem_ALARMS_1_TYPE                                "ALARMS_1_TYPE"
#define configItem_ALARMS_1_MAX                                 "ALARMS_1_MAX"
#define configItem_ALARMS_1_THRESHOLD                           "ALARMS_1_THRESHOLD"
#define configItem_ALARMS_1_ABATEMENT                           "ALARMS_1_ABATEMENT"
#define configItem_ALARMS_1_EXECUTE                             "ALARMS_1_EXECUTE"
#define configItem_ALARMS_2_TYPE                                "ALARMS_2_TYPE"
#define configItem_ALARMS_2_MAX                                 "ALARMS_2_MAX"
#define configItem_ALARMS_2_THRESHOLD                           "ALARMS_2_THRESHOLD"
#define configItem_ALARMS_2_ABATEMENT                           "ALARMS_2_ABATEMENT"
#define configItem_ALARMS_2_EXECUTE                             "ALARMS_2_EXECUTE"
#define configItem_ALARMS_3_TYPE                                "ALARMS_3_TYPE"
#define configItem_ALARMS_3_MAX                                 "ALARMS_3_MAX"
#define configItem_ALARMS_3_THRESHOLD                           "ALARMS_3_THRESHOLD"
#define configItem_ALARMS_3_ABATEMENT                           "ALARMS_3_ABATEMENT"
#define configItem_ALARMS_3_EXECUTE                             "ALARMS_3_EXECUTE"
#define configItem_ALARMS_4_TYPE                                "ALARMS_4_TYPE"
#define configItem_ALARMS_4_MAX                                 "ALARMS_4_MAX"
#define configItem_ALARMS_4_THRESHOLD                           "ALARMS_4_THRESHOLD"
#define configItem_ALARMS_4_ABATEMENT                           "ALARMS_4_ABATEMENT"
#define configItem_ALARMS_4_EXECUTE                             "ALARMS_4_EXECUTE"
#define configItem_ALARMS_5_TYPE                                "ALARMS_5_TYPE"
#define configItem_ALARMS_5_MAX                                 "ALARMS_5_MAX"
#define configItem_ALARMS_5_THRESHOLD                           "ALARMS_5_THRESHOLD"
#define configItem_ALARMS_5_ABATEMENT                           "ALARMS_5_ABATEMENT"
#define configItem_ALARMS_5_EXECUTE                             "ALARMS_5_EXECUTE"
#define configItem_ALARMS_6_TYPE                                "ALARMS_6_TYPE"
#define configItem_ALARMS_6_MAX                                 "ALARMS_6_MAX"
#define configItem_ALARMS_6_THRESHOLD                           "ALARMS_6_THRESHOLD"
#define configItem_ALARMS_6_ABATEMENT                           "ALARMS_6_ABATEMENT"
#define configItem_ALARMS_6_EXECUTE                             "ALARMS_6_EXECUTE"
#define configItem_ALARMS_7_TYPE                                "ALARMS_7_TYPE"
#define configItem_ALARMS_7_MAX                                 "ALARMS_7_MAX"
#define configItem_ALARMS_7_THRESHOLD                           "ALARMS_7_THRESHOLD"
#define configItem_ALARMS_7_ABATEMENT                           "ALARMS_7_ABATEMENT"
#define configItem_ALARMS_7_EXECUTE                             "ALARMS_7_EXECUTE"
#define configItem_ALARMS_8_TYPE                                "ALARMS_8_TYPE"
#define configItem_ALARMS_8_MAX                                 "ALARMS_8_MAX"
#define configItem_ALARMS_8_THRESHOLD                           "ALARMS_8_THRESHOLD"
#define configItem_ALARMS_8_ABATEMENT                           "ALARMS_8_ABATEMENT"
#define configItem_ALARMS_8_EXECUTE                             "ALARMS_8_EXECUTE"
#define configItem_ALARMS_9_TYPE                                "ALARMS_9_TYPE"
#define configItem_ALARMS_9_MAX                                 "ALARMS_9_MAX"
#define configItem_ALARMS_9_THRESHOLD                           "ALARMS_9_THRESHOLD"
#define configItem_ALARMS_9_ABATEMENT                           "ALARMS_9_ABATEMENT"
#define configItem_ALARMS_9_EXECUTE                             "ALARMS_9_EXECUTE"
#define configItem_ALARMS_10_TYPE                               "ALARMS_10_TYPE"
#define configItem_ALARMS_10_MAX                                "ALARMS_10_MAX"
#define configItem_ALARMS_10_THRESHOLD                          "ALARMS_10_THRESHOLD"
#define configItem_ALARMS_10_ABATEMENT                          "ALARMS_10_ABATEMENT"
#define configItem_ALARMS_10_EXECUTE                            "ALARMS_10_EXECUTE"

#define configItemFTP_HOST                                      "FTP_HOST" 
#define configItemFTP_USERNAME                                  "FTP_USERNAME" 
#define configItemFTP_PASSWD                                    "FTP_PASSWD" 
#define configItemFTP_REMOTE_DIRECTORY                          "FTP_REMOTE_DIRECTORY" 
#define configItemFTP_USE_PASSIVE                               "FTP_USE_PASSIVE" 
#define configItemFTP_INTERVAL                                  "FTP_INTERVAL"
#define configItemFTP_RULE_1_SOURCE                             "FTP_RULE_1_SOURCE" 
#define configItemFTP_RULE_2_SOURCE                             "FTP_RULE_2_SOURCE" 
#define configItemFTP_RULE_3_SOURCE                             "FTP_RULE_3_SOURCE"
#define configItemFTP_RULE_4_SOURCE                             "FTP_RULE_4_SOURCE"
#define configItemFTP_RULE_5_SOURCE                             "FTP_RULE_5_SOURCE"
#define configItemFTP_RULE_6_SOURCE                             "FTP_RULE_6_SOURCE"
#define configItemFTP_RULE_7_SOURCE                             "FTP_RULE_7_SOURCE"
#define configItemFTP_RULE_8_SOURCE                             "FTP_RULE_8_SOURCE"
#define configItemFTP_RULE_9_SOURCE                             "FTP_RULE_9_SOURCE"
#define configItemFTP_RULE_10_SOURCE                            "FTP_RULE_10_SOURCE"

#define configItemSSH_1_INTERVAL                                "SSH_1_INTERVAL"
#define configItemSSH_1_SOURCE                                  "SSH_1_SOURCE"
#define configItemSSH_1_HOST                                    "SSH_1_HOST"
#define configItemSSH_1_PORT                                    "SSH_1_PORT" 
#define configItemSSH_1_USERNAME                                "SSH_1_USERNAME" 
#define configItemSSH_1_DESTINATION                             "SSH_1_DESTINATION"
#define configItemSSH_2_INTERVAL                                "SSH_2_INTERVAL"
#define configItemSSH_2_SOURCE                                  "SSH_2_SOURCE"
#define configItemSSH_2_HOST                                    "SSH_2_HOST"
#define configItemSSH_2_PORT                                    "SSH_2_PORT" 
#define configItemSSH_2_USERNAME                                "SSH_2_USERNAME" 
#define configItemSSH_2_DESTINATION                             "SSH_2_DESTINATION"
#define configItemSSH_3_INTERVAL                                "SSH_3_INTERVAL"
#define configItemSSH_3_SOURCE                                  "SSH_3_SOURCE"
#define configItemSSH_3_HOST                                    "SSH_3_HOST"
#define configItemSSH_3_PORT                                    "SSH_3_PORT" 
#define configItemSSH_3_USERNAME                                "SSH_3_USERNAME" 
#define configItemSSH_3_DESTINATION                             "SSH_3_DESTINATION"
#define configItemSSH_4_INTERVAL                                "SSH_4_INTERVAL"
#define configItemSSH_4_SOURCE                                  "SSH_4_SOURCE"
#define configItemSSH_4_HOST                                    "SSH_4_HOST" 
#define configItemSSH_4_PORT                                    "SSH_4_PORT" 
#define configItemSSH_4_USERNAME                                "SSH_4_USERNAME" 
#define configItemSSH_4_DESTINATION                             "SSH_4_DESTINATION"
#define configItemSSH_5_INTERVAL                                "SSH_5_INTERVAL"
#define configItemSSH_5_SOURCE                                  "SSH_5_SOURCE"
#define configItemSSH_5_HOST                                    "SSH_5_HOST"
#define configItemSSH_5_PORT                                    "SSH_5_PORT" 
#define configItemSSH_5_USERNAME                                "SSH_5_USERNAME" 
#define configItemSSH_5_DESTINATION                             "SSH_5_DESTINATION"

#define configItemCWOP_APRS_CALL_SIGN                           "CWOP_APRS_CALL_SIGN"
#define configItemCWOP_APRS_SERVER1                             "CWOP_APRS_SERVER1" 
#define configItemCWOP_APRS_PORTNO1                             "CWOP_APRS_PORTNO1" 
#define configItemCWOP_APRS_SERVER2                             "CWOP_APRS_SERVER2" 
#define configItemCWOP_APRS_PORTNO2                             "CWOP_APRS_PORTNO2" 
#define configItemCWOP_APRS_SERVER3                             "CWOP_APRS_SERVER3" 
#define configItemCWOP_APRS_PORTNO3                             "CWOP_APRS_PORTNO3" 
#define configItemCWOP_LATITUDE                                 "CWOP_LATITUDE" 
#define configItemCWOP_LONGITUDE                                "CWOP_LONGITUDE" 
#define configItemCWOP_LOG_WX_PACKET                            "CWOP_LOG_WX_PACKET" 

#define configItemHTTP_WUSTATIONID                              "HTTP_WUSTATIONID"
#define configItemHTTP_WUPASSWD                                 "HTTP_WUPASSWD"
#define configItemHTTP_YOUSTATIONID                             "HTTP_YOUSTATIONID"
#define configItemHTTP_YOUPASSWD                                "HTTP_YOUPASSWD"

#define configItemCAL_MULT_BAROMETER                            "CAL_MULT_BAROMETER"
#define configItemCAL_CONST_BAROMETER                           "CAL_CONST_BAROMETER" 
#define configItemCAL_MULT_PRESSURE                             "CAL_MULT_PRESSURE"
#define configItemCAL_CONST_PRESSURE                            "CAL_CONST_PRESSURE" 
#define configItemCAL_MULT_ALTIMETER                            "CAL_MULT_ALTIMETER" 
#define configItemCAL_CONST_ALTIMETER                           "CAL_CONST_ALTIMETER" 
#define configItemCAL_MULT_INTEMP                               "CAL_MULT_INTEMP" 
#define configItemCAL_CONST_INTEMP                              "CAL_CONST_INTEMP" 
#define configItemCAL_MULT_OUTTEMP                              "CAL_MULT_OUTTEMP" 
#define configItemCAL_CONST_OUTTEMP                             "CAL_CONST_OUTTEMP" 
#define configItemCAL_MULT_INHUMIDITY                           "CAL_MULT_INHUMIDITY" 
#define configItemCAL_CONST_INHUMIDITY                          "CAL_CONST_INHUMIDITY" 
#define configItemCAL_MULT_OUTHUMIDITY                          "CAL_MULT_OUTHUMIDITY" 
#define configItemCAL_CONST_OUTHUMIDITY                         "CAL_CONST_OUTHUMIDITY" 
#define configItemCAL_MULT_WINDSPEED                            "CAL_MULT_WINDSPEED" 
#define configItemCAL_CONST_WINDSPEED                           "CAL_CONST_WINDSPEED" 
#define configItemCAL_MULT_WINDDIR                              "CAL_MULT_WINDDIR" 
#define configItemCAL_CONST_WINDDIR                             "CAL_CONST_WINDDIR" 
#define configItemCAL_MULT_RAIN                                 "CAL_MULT_RAIN" 
#define configItemCAL_CONST_RAIN                                "CAL_CONST_RAIN" 
#define configItemCAL_MULT_RAINRATE                             "CAL_MULT_RAINRATE"
#define configItemCAL_CONST_RAINRATE                            "CAL_CONST_RAINRATE"

#define configItemPROCMON_wviewd                                "PROCMON_wviewd"
#define configItemPROCMON_htmlgend                              "PROCMON_htmlgend"
#define configItemPROCMON_wvalarmd                              "PROCMON_wvalarmd"
#define configItemPROCMON_wvcwopd                               "PROCMON_wvcwopd"
#define configItemPROCMON_wvhttpd                               "PROCMON_wvhttpd"
#define configItemPROCMON_wviewsqld                             "PROCMON_wviewsqld"

// Define the column names for wview-conf.sdb:
#define configCOLUMN_NAME                                       "name"
#define configCOLUMN_VALUE                                      "value"
#define configCOLUMN_DESCRIPTION                                "description"


//  ... API prototypes

//  wvconfigInit: Initialize/Attach to the wview configuration API:
//  "firstProcess" is a BOOL to indicate if this is the first process to call init;
//  Returns: OK or ERROR
extern int wvconfigInit (int firstProcess);

//  wvconfigExit: clean up and detach from the wview configuration API
extern void wvconfigExit (void);

//  wvconfigGetINTValue: retrieve the integer value for this parameter;
//  Returns: integer value
extern int wvconfigGetINTValue (const char* configItem);

//  wvconfigGetDOUBLEValue: retrieve the double value for this parameter;
//  Returns: double value
extern double wvconfigGetDOUBLEValue (const char* configItem);

//  wvconfigGetStringValue: retrieve the string value for this parameter 
//  Returns: const static string reference or NULL
extern const char* wvconfigGetStringValue (const char* configItem);

//  wvconfigGetBooleanValue: retrieve the bool value for this parameter:
//  Assumption: anything other than lowercase "yes" or "1" or "TRUE" 
//              is considered FALSE
//  Returns: TRUE or FALSE or ERROR
extern int wvconfigGetBooleanValue (const char* configItem);

#endif

