<?php
//
//  File:       preload_calibration.php
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

    // Calibration:
    if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "VantagePro")
    {
        $field_Pressure_Type = "BP";
        $field_CAL_MULT_Pressure = SqliteDBGetValue($dbID, 'CAL_MULT_BAROMETER');
        $field_CAL_CONST_Pressure = SqliteDBGetValue($dbID, 'CAL_CONST_BAROMETER');
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WXT510")
    {
        $field_Pressure_Type = "SP";
        $field_CAL_MULT_Pressure = SqliteDBGetValue($dbID, 'CAL_MULT_PRESSURE');
        $field_CAL_CONST_Pressure = SqliteDBGetValue($dbID, 'CAL_CONST_PRESSURE');
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WS-2300")
    {
        $field_Pressure_Type = "SP";
        $field_CAL_MULT_Pressure = SqliteDBGetValue($dbID, 'CAL_MULT_PRESSURE');
        $field_CAL_CONST_Pressure = SqliteDBGetValue($dbID, 'CAL_CONST_PRESSURE');
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WMR918")
    {
        $field_Pressure_Type = "SP";
        $field_CAL_MULT_Pressure = SqliteDBGetValue($dbID, 'CAL_MULT_PRESSURE');
        $field_CAL_CONST_Pressure = SqliteDBGetValue($dbID, 'CAL_CONST_PRESSURE');
    }
    else
    {
        $field_Pressure_Type = "SP";
        $field_CAL_MULT_Pressure = SqliteDBGetValue($dbID, 'CAL_MULT_PRESSURE');
        $field_CAL_CONST_Pressure = SqliteDBGetValue($dbID, 'CAL_CONST_PRESSURE');
    }
    
    $field_CAL_MULT_InTemp                  = SqliteDBGetValue($dbID, 'CAL_MULT_INTEMP');
    $field_CAL_CONST_InTemp                 = SqliteDBGetValue($dbID, 'CAL_CONST_INTEMP');
    $field_CAL_MULT_OutTemp                 = SqliteDBGetValue($dbID, 'CAL_MULT_OUTTEMP');
    $field_CAL_CONST_OutTemp                = SqliteDBGetValue($dbID, 'CAL_CONST_OUTTEMP');
    $field_CAL_MULT_InHumidity              = SqliteDBGetValue($dbID, 'CAL_MULT_INHUMIDITY');
    $field_CAL_CONST_InHumidity             = SqliteDBGetValue($dbID, 'CAL_CONST_INHUMIDITY');
    $field_CAL_MULT_OutHumidity             = SqliteDBGetValue($dbID, 'CAL_MULT_OUTHUMIDITY');
    $field_CAL_CONST_OutHumidity            = SqliteDBGetValue($dbID, 'CAL_CONST_OUTHUMIDITY');
    $field_CAL_MULT_Windspeed               = SqliteDBGetValue($dbID, 'CAL_MULT_WINDSPEED');
    $field_CAL_CONST_Windspeed              = SqliteDBGetValue($dbID, 'CAL_CONST_WINDSPEED');
    $field_CAL_MULT_Windir                  = SqliteDBGetValue($dbID, 'CAL_MULT_WINDDIR');
    $field_CAL_CONST_Windir                 = SqliteDBGetValue($dbID, 'CAL_CONST_WINDDIR');
    $field_CAL_MULT_Rain                    = SqliteDBGetValue($dbID, 'CAL_MULT_RAIN');
    $field_CAL_CONST_Rain                   = SqliteDBGetValue($dbID, 'CAL_CONST_RAIN');
    $field_CAL_MULT_Rainrate                = SqliteDBGetValue($dbID, 'CAL_MULT_RAINRATE');
    $field_CAL_CONST_Rainrate               = SqliteDBGetValue($dbID, 'CAL_CONST_RAINRATE');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
