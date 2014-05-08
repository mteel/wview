<?php
//
//  File:       preload_ftp.php
//
//  Purpose:    Preload config items from the SQLite3 database.
//

    // Open the database:
    $dbID = SqliteDBOpen();

    // Get the version number:
    $filestr = GetConfigPrefix() . "/wview/wview-version";
    $wview_version = file_get_contents($filestr);

    // Station:
    $stationType = SqliteDBGetValue($dbID, 'STATION_TYPE');
    if ($stationType == "VantagePro")
    {
        $field_Station_Type = "Davis Vantage Pro";
    }
    else if ($stationType == "WXT510")
    {
        $field_Station_Type = "Viasala WXT510";
    }
    else if ($stationType == "TWI")
    {
        $field_Station_Type = "Texas Weather Instruments";
    }
    else if ($stationType == "WS-2300")
    {
        $field_Station_Type = "La Crosse WS-23XX";
    }
    else if ($stationType == "WMR918")
    {
        $field_Station_Type = "Oregon Scientific WMR9XX";
    }
    else if ($stationType == "WMRUSB")
    {
        $field_Station_Type = "Oregon Scientific WMRUSB";
    }
    else if ($stationType == "WH1080")
    {
        $field_Station_Type = "Fine Offset WH1080";
    }
    else if ($stationType == "TE923")
    {
        $field_Station_Type = "Honeywell TE923";
    }
    else if ($stationType == "Simulator")
    {
        $field_Station_Type = "Simulator";
    }
    else if ($stationType == "Virtual")
    {
        $field_Station_Type = "Virtual";
    }

    // FTP:
    $field_FTP_Hostname                     = SqliteDBGetValue($dbID, 'FTP_HOST');
    $field_FTP_Username                     = SqliteDBGetValue($dbID, 'FTP_USERNAME');
    $field_FTP_Password                     = SqliteDBGetValue($dbID, 'FTP_PASSWD');
    $field_FTP_Remote_Dir                   = SqliteDBGetValue($dbID, 'FTP_REMOTE_DIRECTORY');
    $field_FTP_Use_Passive                  = SqliteDBGetValue($dbID, 'FTP_USE_PASSIVE');
    $field_FTP_Interval                     = SqliteDBGetValue($dbID, 'FTP_INTERVAL');

    $field_FTP_Source_1                     = SqliteDBGetValue($dbID, 'FTP_RULE_1_SOURCE');
    $field_FTP_Source_2                     = SqliteDBGetValue($dbID, 'FTP_RULE_2_SOURCE');
    $field_FTP_Source_3                     = SqliteDBGetValue($dbID, 'FTP_RULE_3_SOURCE');
    $field_FTP_Source_4                     = SqliteDBGetValue($dbID, 'FTP_RULE_4_SOURCE');
    $field_FTP_Source_5                     = SqliteDBGetValue($dbID, 'FTP_RULE_5_SOURCE');
    $field_FTP_Source_6                     = SqliteDBGetValue($dbID, 'FTP_RULE_6_SOURCE');
    $field_FTP_Source_7                     = SqliteDBGetValue($dbID, 'FTP_RULE_7_SOURCE');
    $field_FTP_Source_8                     = SqliteDBGetValue($dbID, 'FTP_RULE_8_SOURCE');
    $field_FTP_Source_9                     = SqliteDBGetValue($dbID, 'FTP_RULE_9_SOURCE');
    $field_FTP_Source_10                    = SqliteDBGetValue($dbID, 'FTP_RULE_10_SOURCE');
    
    // Close the database connection:
    SqliteDBClose($dbID);

?>
