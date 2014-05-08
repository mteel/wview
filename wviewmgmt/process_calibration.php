<?php
//
//  File:       process_calibration.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // Calibration:
    if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "VantagePro")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_BAROMETER', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_BAROMETER', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WXT510")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WS-2300")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WMR918")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "TE923")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_BAROMETER', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_BAROMETER', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "TWI")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_BAROMETER', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_BAROMETER', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WMRUSB")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    else if (SqliteDBGetValue($dbID, 'STATION_TYPE') == "WH1080")
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    else
    {
        SqliteDBSetValue($dbID, 'CAL_MULT_PRESSURE', $_POST['field_CAL_MULT_Pressure']);
        SqliteDBSetValue($dbID, 'CAL_CONST_PRESSURE', $_POST['field_CAL_CONST_Pressure']);
    }
    
    SqliteDBSetValue($dbID, 'CAL_MULT_INTEMP', $_POST['field_CAL_MULT_InTemp']);
    SqliteDBSetValue($dbID, 'CAL_CONST_INTEMP', $_POST['field_CAL_CONST_InTemp']);
    SqliteDBSetValue($dbID, 'CAL_MULT_OUTTEMP', $_POST['field_CAL_MULT_OutTemp']);
    SqliteDBSetValue($dbID, 'CAL_CONST_OUTTEMP', $_POST['field_CAL_CONST_OutTemp']);
    SqliteDBSetValue($dbID, 'CAL_MULT_INHUMIDITY', $_POST['field_CAL_MULT_InHumidity']);
    SqliteDBSetValue($dbID, 'CAL_CONST_INHUMIDITY', $_POST['field_CAL_CONST_InHumidity']);
    SqliteDBSetValue($dbID, 'CAL_MULT_OUTHUMIDITY', $_POST['field_CAL_MULT_OutHumidity']);
    SqliteDBSetValue($dbID, 'CAL_CONST_OUTHUMIDITY', $_POST['field_CAL_CONST_OutHumidity']);
    SqliteDBSetValue($dbID, 'CAL_MULT_WINDSPEED', $_POST['field_CAL_MULT_Windspeed']);
    SqliteDBSetValue($dbID, 'CAL_CONST_WINDSPEED', $_POST['field_CAL_CONST_Windspeed']);
    SqliteDBSetValue($dbID, 'CAL_MULT_WINDDIR', $_POST['field_CAL_MULT_Windir']);
    SqliteDBSetValue($dbID, 'CAL_CONST_WINDDIR', $_POST['field_CAL_CONST_Windir']);
    SqliteDBSetValue($dbID, 'CAL_MULT_RAIN', $_POST['field_CAL_MULT_Rain']);
    SqliteDBSetValue($dbID, 'CAL_CONST_RAIN', $_POST['field_CAL_CONST_Rain']);
    SqliteDBSetValue($dbID, 'CAL_MULT_RAINRATE', $_POST['field_CAL_MULT_Rainrate']);
    SqliteDBSetValue($dbID, 'CAL_CONST_RAINRATE', $_POST['field_CAL_CONST_Rainrate']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'calibration.php');

?>