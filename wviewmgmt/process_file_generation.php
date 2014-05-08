<?php
//
//  File:       process_file_generation.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // Save all config items to the database:

    SqliteDBSetValue($dbID, 'HTMLGEN_STATION_NAME', $_POST['field_Generate_Station_Name']);
    SqliteDBSetValue($dbID, 'HTMLGEN_STATION_CITY', $_POST['field_Generate_Station_City']);
    SqliteDBSetValue($dbID, 'HTMLGEN_STATION_STATE', $_POST['field_Generate_Station_State']);
    if ($_POST['field_Generate_Station_IF'][0] == "yes")
        SqliteDBSetValue($dbID, 'HTMLGEN_STATION_SHOW_IF', 'yes');
    else
        SqliteDBSetValue($dbID, 'HTMLGEN_STATION_SHOW_IF', 'no');
    SqliteDBSetValue($dbID, 'HTMLGEN_IMAGE_PATH', $_POST['field_Generate_Target']);
    SqliteDBSetValue($dbID, 'HTMLGEN_HTML_PATH', $_POST['field_Generate_Source']);
    SqliteDBSetValue($dbID, 'HTMLGEN_START_OFFSET', $_POST['field_Generate_Start_Offset']);
    SqliteDBSetValue($dbID, 'HTMLGEN_GENERATE_INTERVAL', $_POST['field_Generate_Interval']);
    if ($_POST['field_Generate_Metric'][0] == "yes")
        SqliteDBSetValue($dbID, 'HTMLGEN_METRIC_UNITS', 'yes');
    else
        SqliteDBSetValue($dbID, 'HTMLGEN_METRIC_UNITS', 'no');
    if ($_POST['field_Generate_Metric_MM'][0] == "yes")
        SqliteDBSetValue($dbID, 'HTMLGEN_METRIC_USE_RAIN_MM', 'yes');
    else
        SqliteDBSetValue($dbID, 'HTMLGEN_METRIC_USE_RAIN_MM', 'no');
    SqliteDBSetValue($dbID, 'HTMLGEN_WIND_UNITS', $_POST['field_Generate_WindUnits']);
    if ($_POST['field_Generate_Dual_Units'][0] == "yes")
        SqliteDBSetValue($dbID, 'HTMLGEN_DUAL_UNITS', 'yes');
    else
        SqliteDBSetValue($dbID, 'HTMLGEN_DUAL_UNITS', 'no');
    if ($_POST['field_Generate_Extended'][0] == "yes")
        SqliteDBSetValue($dbID, 'HTMLGEN_EXTENDED_DATA', 'yes');
    else
        SqliteDBSetValue($dbID, 'HTMLGEN_EXTENDED_DATA', 'no');
    SqliteDBSetValue($dbID, 'HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP', $_POST['field_Generate_Archive_Days']);
    SqliteDBSetValue($dbID, 'HTMLGEN_MPHASE_INCREASE', $_POST['field_Generate_Moon_Increasing']);
    SqliteDBSetValue($dbID, 'HTMLGEN_MPHASE_DECREASE', $_POST['field_Generate_Moon_Decreasing']);
    SqliteDBSetValue($dbID, 'HTMLGEN_MPHASE_FULL', $_POST['field_Generate_Moon_Full']);
    SqliteDBSetValue($dbID, 'HTMLGEN_LOCAL_RADAR_URL', $_POST['field_Generate_Radar_URL']);
    SqliteDBSetValue($dbID, 'HTMLGEN_LOCAL_FORECAST_URL', $_POST['field_Generate_Forecast_URL']);
    SqliteDBSetValue($dbID, 'HTMLGEN_DATE_FORMAT', $_POST['field_Generate_Date_Format']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'file_generation.php');

?>