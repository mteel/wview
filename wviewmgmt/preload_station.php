<?php
//
//  File:       preload_station.php
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

    $field_Station_Interface                = SqliteDBGetValue($dbID, 'STATION_INTERFACE');
    $field_Station_Device                   = SqliteDBGetValue($dbID, 'STATION_DEV');
    $field_Station_Host                     = SqliteDBGetValue($dbID, 'STATION_HOST');
    $field_Station_Port                     = SqliteDBGetValue($dbID, 'STATION_PORT');
    $field_Station_WLIP                     = SqliteDBGetValue($dbID, 'STATION_WLIP');
    $field_Station_Retrieve_Archive         = SqliteDBGetValue($dbID, 'STATION_RETRIEVE_ARCHIVE');
    $field_Station_DTR                      = SqliteDBGetValue($dbID, 'STATION_DTR');
    $field_Station_Rain_Season_Start        = SqliteDBGetValue($dbID, 'STATION_RAIN_SEASON_START');
    $field_Station_Storm_Trigger_Start      = SqliteDBGetValue($dbID, 'STATION_RAIN_STORM_TRIGGER_START');
    $field_Station_Storm_Trigger_Stop       = SqliteDBGetValue($dbID, 'STATION_RAIN_STORM_IDLE_STOP');
    $field_Station_Rain_YTD                 = SqliteDBGetValue($dbID, 'STATION_RAIN_YTD');
    $field_Station_ET_YTD                   = SqliteDBGetValue($dbID, 'STATION_ET_YTD');
    $field_Station_YTD_Year                 = SqliteDBGetValue($dbID, 'STATION_RAIN_ET_YTD_YEAR');
    $field_Station_Elevation                = SqliteDBGetValue($dbID, 'STATION_ELEVATION');
    $field_Station_Latitude                 = SqliteDBGetValue($dbID, 'STATION_LATITUDE');
    $field_Station_Longitude                = SqliteDBGetValue($dbID, 'STATION_LONGITUDE');
    $field_Station_Archive_Interval         = SqliteDBGetValue($dbID, 'STATION_ARCHIVE_INTERVAL');
    $field_Station_Polling_Interval         = SqliteDBGetValue($dbID, 'STATION_POLL_INTERVAL');
    $field_Station_Push_Interval            = SqliteDBGetValue($dbID, 'STATION_PUSH_INTERVAL');
    $field_Station_Do_RX_Check              = SqliteDBGetValue($dbID, 'STATION_DO_RCHECK');
    $field_Station_Outside_Channel          = SqliteDBGetValue($dbID, 'STATION_OUTSIDE_CHANNEL');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
