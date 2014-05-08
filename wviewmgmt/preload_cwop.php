<?php
//
//  File:       preload_cwop.php
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

    // CWOP:
    $field_CWOP_CallSign                    = SqliteDBGetValue($dbID, 'CWOP_APRS_CALL_SIGN');
    $field_CWOP_Server_1                    = SqliteDBGetValue($dbID, 'CWOP_APRS_SERVER1');
    $field_CWOP_Port_1                      = SqliteDBGetValue($dbID, 'CWOP_APRS_PORTNO1');
    $field_CWOP_Server_2                    = SqliteDBGetValue($dbID, 'CWOP_APRS_SERVER2');
    $field_CWOP_Port_2                      = SqliteDBGetValue($dbID, 'CWOP_APRS_PORTNO2');
    $field_CWOP_Server_3                    = SqliteDBGetValue($dbID, 'CWOP_APRS_SERVER3');
    $field_CWOP_Port_3                      = SqliteDBGetValue($dbID, 'CWOP_APRS_PORTNO3');
    $field_CWOP_Latitude                    = SqliteDBGetValue($dbID, 'CWOP_LATITUDE');
    $field_CWOP_Longitude                   = SqliteDBGetValue($dbID, 'CWOP_LONGITUDE');
    $field_CWOP_Log_Packet                  = SqliteDBGetValue($dbID, 'CWOP_LOG_WX_PACKET');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
