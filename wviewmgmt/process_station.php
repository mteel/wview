<?php
//
//  File:       process_station.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // Station:
    if ($_POST['field_Station_Type'] == "Davis Vantage Pro")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'VantagePro');
        $systemstr = "echo  \"wviewd_vpro\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Viasala WXT510")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'WXT510');
        $systemstr = "echo  \"wviewd_wxt510\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Texas Weather Instruments")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'TWI');
        $systemstr = "echo  \"wviewd_twi\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "La Crosse WS-23XX")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'WS-2300');
        $systemstr = "echo  \"wviewd_ws2300\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Oregon Scientific WMR9XX")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'WMR918');
        $systemstr = "echo  \"wviewd_wmr918\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Oregon Scientific WMRUSB")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'WMRUSB');
        $systemstr = "echo  \"wviewd_wmrusb\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Fine Offset WH1080")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'WH1080');
        $systemstr = "echo  \"wviewd_wh1080\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Honeywell TE923")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'TE923');
        $systemstr = "echo  \"wviewd_te923\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Simulator")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'Simulator');
        $systemstr = "echo  \"wviewd_sim\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else if ($_POST['field_Station_Type'] == "Virtual")
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'Virtual');
        $systemstr = "echo  \"wviewd_virtual\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }
    else
    {
        SqliteDBSetValue($dbID, 'STATION_TYPE', 'Simulator');
        $systemstr = "echo  \"wviewd_sim\" > " . GetConfigPrefix() . "/wview/wview-binary";
        system($systemstr);
    }

    SqliteDBSetValue($dbID, 'STATION_INTERFACE', $_POST['field_Station_Interface']);
    SqliteDBSetValue($dbID, 'STATION_DEV', $_POST['field_Station_Device']);
    SqliteDBSetValue($dbID, 'STATION_HOST', $_POST['field_Station_Host']);
    SqliteDBSetValue($dbID, 'STATION_PORT', $_POST['field_Station_Port']);
    if ($_POST['field_Station_WLIP'][0] == "yes")
        SqliteDBSetValue($dbID, 'STATION_WLIP', 'yes');
    else
        SqliteDBSetValue($dbID, 'STATION_WLIP', 'no');
    if ($_POST['field_Station_Retrieve_Archive'][0] == "yes")
        SqliteDBSetValue($dbID, 'STATION_RETRIEVE_ARCHIVE', 'yes');
    else
        SqliteDBSetValue($dbID, 'STATION_RETRIEVE_ARCHIVE', 'no');
    if ($_POST['field_Station_Do_DTR'][0] == "yes")
        SqliteDBSetValue($dbID, 'STATION_DTR', 'yes');
    else
        SqliteDBSetValue($dbID, 'STATION_DTR', 'no');
    SqliteDBSetValue($dbID, 'STATION_RAIN_SEASON_START', $_POST['field_Station_Rain_Season_Start']);
    SqliteDBSetValue($dbID, 'STATION_RAIN_STORM_TRIGGER_START', $_POST['field_Station_Storm_Trigger_Start']);
    SqliteDBSetValue($dbID, 'STATION_RAIN_STORM_IDLE_STOP', $_POST['field_Station_Storm_Trigger_Stop']);
    SqliteDBSetValue($dbID, 'STATION_RAIN_YTD', $_POST['field_Station_Rain_YTD']);
    SqliteDBSetValue($dbID, 'STATION_ET_YTD', $_POST['field_Station_ET_YTD']);
    SqliteDBSetValue($dbID, 'STATION_RAIN_ET_YTD_YEAR', $_POST['field_Station_YTD_Year']);
    SqliteDBSetValue($dbID, 'STATION_ELEVATION', $_POST['field_Station_Elevation']);
    SqliteDBSetValue($dbID, 'STATION_LATITUDE', $_POST['field_Station_Latitude']);
    SqliteDBSetValue($dbID, 'STATION_LONGITUDE', $_POST['field_Station_Longitude']);
    SqliteDBSetValue($dbID, 'STATION_ARCHIVE_INTERVAL', $_POST['field_Station_Archive_Interval']);
    SqliteDBSetValue($dbID, 'STATION_POLL_INTERVAL', $_POST['field_Station_Polling_Interval']);
    SqliteDBSetValue($dbID, 'STATION_PUSH_INTERVAL', $_POST['field_Station_Push_Interval']);
    if ($_POST['field_Station_Do_RX_Check'][0] == "yes")
        SqliteDBSetValue($dbID, 'STATION_DO_RCHECK', 'yes');
    else
        SqliteDBSetValue($dbID, 'STATION_DO_RCHECK', 'no');
    SqliteDBSetValue($dbID, 'STATION_OUTSIDE_CHANNEL', $_POST['field_Station_Outside_Channel']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'station.php');

?>
