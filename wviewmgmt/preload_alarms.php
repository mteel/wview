<?php
//
//  File:       preload_alarms.php
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

    // Alarms:
    $field_Alarms_Metric                    = SqliteDBGetValue($dbID, 'ALARMS_STATION_METRIC');
    $field_Alarms_Do_Test                   = SqliteDBGetValue($dbID, 'ALARMS_DO_TEST');
    $field_Alarms_Do_Test_Number            = SqliteDBGetValue($dbID, 'ALARMS_DO_TEST_NUMBER');

    $field_Alarms_Type_1                    = SqliteDBGetValue($dbID, 'ALARMS_1_TYPE');
    $field_Alarms_Max_1                     = SqliteDBGetValue($dbID, 'ALARMS_1_MAX');
    $field_Alarms_Threshold_1               = SqliteDBGetValue($dbID, 'ALARMS_1_THRESHOLD');
    $field_Alarms_Abatement_1               = SqliteDBGetValue($dbID, 'ALARMS_1_ABATEMENT');
    $field_Alarms_Execute_1                 = SqliteDBGetValue($dbID, 'ALARMS_1_EXECUTE');
    $field_Alarms_Type_2                    = SqliteDBGetValue($dbID, 'ALARMS_2_TYPE');
    $field_Alarms_Max_2                     = SqliteDBGetValue($dbID, 'ALARMS_2_MAX');
    $field_Alarms_Threshold_2               = SqliteDBGetValue($dbID, 'ALARMS_2_THRESHOLD');
    $field_Alarms_Abatement_2               = SqliteDBGetValue($dbID, 'ALARMS_2_ABATEMENT');
    $field_Alarms_Execute_2                 = SqliteDBGetValue($dbID, 'ALARMS_2_EXECUTE');
    $field_Alarms_Type_3                    = SqliteDBGetValue($dbID, 'ALARMS_3_TYPE');
    $field_Alarms_Max_3                     = SqliteDBGetValue($dbID, 'ALARMS_3_MAX');
    $field_Alarms_Threshold_3               = SqliteDBGetValue($dbID, 'ALARMS_3_THRESHOLD');
    $field_Alarms_Abatement_3               = SqliteDBGetValue($dbID, 'ALARMS_3_ABATEMENT');
    $field_Alarms_Execute_3                 = SqliteDBGetValue($dbID, 'ALARMS_3_EXECUTE');
    $field_Alarms_Type_4                    = SqliteDBGetValue($dbID, 'ALARMS_4_TYPE');
    $field_Alarms_Max_4                     = SqliteDBGetValue($dbID, 'ALARMS_4_MAX');
    $field_Alarms_Threshold_4               = SqliteDBGetValue($dbID, 'ALARMS_4_THRESHOLD');
    $field_Alarms_Abatement_4               = SqliteDBGetValue($dbID, 'ALARMS_4_ABATEMENT');
    $field_Alarms_Execute_4                 = SqliteDBGetValue($dbID, 'ALARMS_4_EXECUTE');
    $field_Alarms_Type_5                    = SqliteDBGetValue($dbID, 'ALARMS_5_TYPE');
    $field_Alarms_Max_5                     = SqliteDBGetValue($dbID, 'ALARMS_5_MAX');
    $field_Alarms_Threshold_5               = SqliteDBGetValue($dbID, 'ALARMS_5_THRESHOLD');
    $field_Alarms_Abatement_5               = SqliteDBGetValue($dbID, 'ALARMS_5_ABATEMENT');
    $field_Alarms_Execute_5                 = SqliteDBGetValue($dbID, 'ALARMS_5_EXECUTE');
    $field_Alarms_Type_6                    = SqliteDBGetValue($dbID, 'ALARMS_6_TYPE');
    $field_Alarms_Max_6                     = SqliteDBGetValue($dbID, 'ALARMS_6_MAX');
    $field_Alarms_Threshold_6               = SqliteDBGetValue($dbID, 'ALARMS_6_THRESHOLD');
    $field_Alarms_Abatement_6               = SqliteDBGetValue($dbID, 'ALARMS_6_ABATEMENT');
    $field_Alarms_Execute_6                 = SqliteDBGetValue($dbID, 'ALARMS_6_EXECUTE');
    $field_Alarms_Type_7                    = SqliteDBGetValue($dbID, 'ALARMS_7_TYPE');
    $field_Alarms_Max_7                     = SqliteDBGetValue($dbID, 'ALARMS_7_MAX');
    $field_Alarms_Threshold_7               = SqliteDBGetValue($dbID, 'ALARMS_7_THRESHOLD');
    $field_Alarms_Abatement_7               = SqliteDBGetValue($dbID, 'ALARMS_7_ABATEMENT');
    $field_Alarms_Execute_7                 = SqliteDBGetValue($dbID, 'ALARMS_7_EXECUTE');
    $field_Alarms_Type_8                    = SqliteDBGetValue($dbID, 'ALARMS_8_TYPE');
    $field_Alarms_Max_8                     = SqliteDBGetValue($dbID, 'ALARMS_8_MAX');
    $field_Alarms_Threshold_8               = SqliteDBGetValue($dbID, 'ALARMS_8_THRESHOLD');
    $field_Alarms_Abatement_8               = SqliteDBGetValue($dbID, 'ALARMS_8_ABATEMENT');
    $field_Alarms_Execute_8                 = SqliteDBGetValue($dbID, 'ALARMS_8_EXECUTE');
    $field_Alarms_Type_9                    = SqliteDBGetValue($dbID, 'ALARMS_9_TYPE');
    $field_Alarms_Max_9                     = SqliteDBGetValue($dbID, 'ALARMS_9_MAX');
    $field_Alarms_Threshold_9               = SqliteDBGetValue($dbID, 'ALARMS_9_THRESHOLD');
    $field_Alarms_Abatement_9               = SqliteDBGetValue($dbID, 'ALARMS_9_ABATEMENT');
    $field_Alarms_Execute_9                 = SqliteDBGetValue($dbID, 'ALARMS_9_EXECUTE');
    $field_Alarms_Type_10                   = SqliteDBGetValue($dbID, 'ALARMS_10_TYPE');
    $field_Alarms_Max_10                    = SqliteDBGetValue($dbID, 'ALARMS_10_MAX');
    $field_Alarms_Threshold_10              = SqliteDBGetValue($dbID, 'ALARMS_10_THRESHOLD');
    $field_Alarms_Abatement_10              = SqliteDBGetValue($dbID, 'ALARMS_10_ABATEMENT');
    $field_Alarms_Execute_10                = SqliteDBGetValue($dbID, 'ALARMS_10_EXECUTE');

    // Close the database connection:
    SqliteDBClose($dbID);

?>
