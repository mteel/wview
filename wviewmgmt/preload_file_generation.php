<?php
//
//  File:       preload.php
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

    // Generation:
    $field_Generate_Station_Name            = SqliteDBGetValue($dbID, 'HTMLGEN_STATION_NAME');
    $field_Generate_Station_City            = SqliteDBGetValue($dbID, 'HTMLGEN_STATION_CITY');
    $field_Generate_Station_State           = SqliteDBGetValue($dbID, 'HTMLGEN_STATION_STATE');
    $field_Generate_Station_IF              = SqliteDBGetValue($dbID, 'HTMLGEN_STATION_SHOW_IF');
    $field_Generate_Target                  = SqliteDBGetValue($dbID, 'HTMLGEN_IMAGE_PATH');
    $field_Generate_Source                  = SqliteDBGetValue($dbID, 'HTMLGEN_HTML_PATH');
    $field_Generate_Start_Offset            = SqliteDBGetValue($dbID, 'HTMLGEN_START_OFFSET');
    $field_Generate_Interval                = SqliteDBGetValue($dbID, 'HTMLGEN_GENERATE_INTERVAL');
    $field_Generate_Metric                  = SqliteDBGetValue($dbID, 'HTMLGEN_METRIC_UNITS');
    $field_Generate_Metric_MM               = SqliteDBGetValue($dbID, 'HTMLGEN_METRIC_USE_RAIN_MM');
    $field_Generate_WindUnits               = SqliteDBGetValue($dbID, 'HTMLGEN_WIND_UNITS');
    $field_Generate_Dual_Units              = SqliteDBGetValue($dbID, 'HTMLGEN_DUAL_UNITS');
    $field_Generate_Extended                = SqliteDBGetValue($dbID, 'HTMLGEN_EXTENDED_DATA');
    $field_Generate_Archive_Days            = SqliteDBGetValue($dbID, 'HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP');
    $field_Generate_Moon_Increasing         = SqliteDBGetValue($dbID, 'HTMLGEN_MPHASE_INCREASE');
    $field_Generate_Moon_Decreasing         = SqliteDBGetValue($dbID, 'HTMLGEN_MPHASE_DECREASE');
    $field_Generate_Moon_Full               = SqliteDBGetValue($dbID, 'HTMLGEN_MPHASE_FULL');
    $field_Generate_Radar_URL               = SqliteDBGetValue($dbID, 'HTMLGEN_LOCAL_RADAR_URL');
    $field_Generate_Forecast_URL            = SqliteDBGetValue($dbID, 'HTMLGEN_LOCAL_FORECAST_URL');
    $field_Generate_Date_Format             = SqliteDBGetValue($dbID, 'HTMLGEN_DATE_FORMAT');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
