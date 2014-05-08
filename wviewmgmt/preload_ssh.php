<?php
//
//  File:       preload_ssh.php
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

    // SSH:
    $field_SSH_Source_1                     = SqliteDBGetValue($dbID, 'SSH_1_SOURCE');
    $field_SSH_Interval_1                   = SqliteDBGetValue($dbID, 'SSH_1_INTERVAL');
    $field_SSH_Host_1                       = SqliteDBGetValue($dbID, 'SSH_1_HOST');
    $field_SSH_Port_1                       = SqliteDBGetValue($dbID, 'SSH_1_PORT');
    $field_SSH_UserName_1                   = SqliteDBGetValue($dbID, 'SSH_1_USERNAME');
    $field_SSH_Destination_1                = SqliteDBGetValue($dbID, 'SSH_1_DESTINATION');
    $field_SSH_Timeout_1                    = SqliteDBGetValue($dbID, 'SSH_1_TIMEOUT');
    $field_SSH_Source_2                     = SqliteDBGetValue($dbID, 'SSH_2_SOURCE');
    $field_SSH_Interval_2                   = SqliteDBGetValue($dbID, 'SSH_2_INTERVAL');
    $field_SSH_Host_2                       = SqliteDBGetValue($dbID, 'SSH_2_HOST');
    $field_SSH_Port_2                       = SqliteDBGetValue($dbID, 'SSH_2_PORT');
    $field_SSH_UserName_2                   = SqliteDBGetValue($dbID, 'SSH_2_USERNAME');
    $field_SSH_Destination_2                = SqliteDBGetValue($dbID, 'SSH_2_DESTINATION');
    $field_SSH_Timeout_2                    = SqliteDBGetValue($dbID, 'SSH_2_TIMEOUT');
    $field_SSH_Source_3                     = SqliteDBGetValue($dbID, 'SSH_3_SOURCE');
    $field_SSH_Interval_3                   = SqliteDBGetValue($dbID, 'SSH_3_INTERVAL');
    $field_SSH_Host_3                       = SqliteDBGetValue($dbID, 'SSH_3_HOST');
    $field_SSH_Port_3                       = SqliteDBGetValue($dbID, 'SSH_3_PORT');
    $field_SSH_UserName_3                   = SqliteDBGetValue($dbID, 'SSH_3_USERNAME');
    $field_SSH_Destination_3                = SqliteDBGetValue($dbID, 'SSH_3_DESTINATION');
    $field_SSH_Timeout_3                    = SqliteDBGetValue($dbID, 'SSH_3_TIMEOUT');
    $field_SSH_Source_4                     = SqliteDBGetValue($dbID, 'SSH_4_SOURCE');
    $field_SSH_Interval_4                   = SqliteDBGetValue($dbID, 'SSH_4_INTERVAL');
    $field_SSH_Host_4                       = SqliteDBGetValue($dbID, 'SSH_4_HOST');
    $field_SSH_Port_4                       = SqliteDBGetValue($dbID, 'SSH_4_PORT');
    $field_SSH_UserName_4                   = SqliteDBGetValue($dbID, 'SSH_4_USERNAME');
    $field_SSH_Destination_4                = SqliteDBGetValue($dbID, 'SSH_4_DESTINATION');
    $field_SSH_Timeout_4                    = SqliteDBGetValue($dbID, 'SSH_4_TIMEOUT');
    $field_SSH_Source_5                     = SqliteDBGetValue($dbID, 'SSH_5_SOURCE');
    $field_SSH_Interval_5                   = SqliteDBGetValue($dbID, 'SSH_5_INTERVAL');
    $field_SSH_Host_5                       = SqliteDBGetValue($dbID, 'SSH_5_HOST');
    $field_SSH_Port_5                       = SqliteDBGetValue($dbID, 'SSH_5_PORT');
    $field_SSH_UserName_5                   = SqliteDBGetValue($dbID, 'SSH_5_USERNAME');
    $field_SSH_Destination_5                = SqliteDBGetValue($dbID, 'SSH_5_DESTINATION');
    $field_SSH_Timeout_5                    = SqliteDBGetValue($dbID, 'SSH_5_TIMEOUT');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
