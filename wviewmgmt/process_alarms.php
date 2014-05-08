<?php
//
//  File:       process_alarms.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // Save all config items to the database:

    if ($_POST['field_Alarms_Metric'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_STATION_METRIC', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_STATION_METRIC', 'no');
    if ($_POST['field_Alarms_Do_Test'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_DO_TEST', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_DO_TEST', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_DO_TEST_NUMBER', $_POST['field_Alarms_Do_Test_Number']);

    SqliteDBSetValue($dbID, 'ALARMS_1_TYPE', $_POST['field_Alarms_Type_1']);
    if ($_POST['field_Alarms_Max_1'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_1_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_1_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_1_THRESHOLD', $_POST['field_Alarms_Threshold_1']);
    SqliteDBSetValue($dbID, 'ALARMS_1_ABATEMENT', $_POST['field_Alarms_Abatement_1']);
    SqliteDBSetValue($dbID, 'ALARMS_1_EXECUTE', $_POST['field_Alarms_Execute_1']);

    SqliteDBSetValue($dbID, 'ALARMS_2_TYPE', $_POST['field_Alarms_Type_2']);
    if ($_POST['field_Alarms_Max_2'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_2_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_2_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_2_THRESHOLD', $_POST['field_Alarms_Threshold_2']);
    SqliteDBSetValue($dbID, 'ALARMS_2_ABATEMENT', $_POST['field_Alarms_Abatement_2']);
    SqliteDBSetValue($dbID, 'ALARMS_2_EXECUTE', $_POST['field_Alarms_Execute_2']);

    SqliteDBSetValue($dbID, 'ALARMS_3_TYPE', $_POST['field_Alarms_Type_3']);
    if ($_POST['field_Alarms_Max_3'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_3_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_3_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_3_THRESHOLD', $_POST['field_Alarms_Threshold_3']);
    SqliteDBSetValue($dbID, 'ALARMS_3_ABATEMENT', $_POST['field_Alarms_Abatement_3']);
    SqliteDBSetValue($dbID, 'ALARMS_3_EXECUTE', $_POST['field_Alarms_Execute_3']);

    SqliteDBSetValue($dbID, 'ALARMS_4_TYPE', $_POST['field_Alarms_Type_4']);
    if ($_POST['field_Alarms_Max_4'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_4_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_4_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_4_THRESHOLD', $_POST['field_Alarms_Threshold_4']);
    SqliteDBSetValue($dbID, 'ALARMS_4_ABATEMENT', $_POST['field_Alarms_Abatement_4']);
    SqliteDBSetValue($dbID, 'ALARMS_4_EXECUTE', $_POST['field_Alarms_Execute_4']);

    SqliteDBSetValue($dbID, 'ALARMS_5_TYPE', $_POST['field_Alarms_Type_5']);
    if ($_POST['field_Alarms_Max_5'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_5_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_5_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_5_THRESHOLD', $_POST['field_Alarms_Threshold_5']);
    SqliteDBSetValue($dbID, 'ALARMS_5_ABATEMENT', $_POST['field_Alarms_Abatement_5']);
    SqliteDBSetValue($dbID, 'ALARMS_5_EXECUTE', $_POST['field_Alarms_Execute_5']);

    SqliteDBSetValue($dbID, 'ALARMS_6_TYPE', $_POST['field_Alarms_Type_6']);
    if ($_POST['field_Alarms_Max_6'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_6_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_6_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_6_THRESHOLD', $_POST['field_Alarms_Threshold_6']);
    SqliteDBSetValue($dbID, 'ALARMS_6_ABATEMENT', $_POST['field_Alarms_Abatement_6']);
    SqliteDBSetValue($dbID, 'ALARMS_6_EXECUTE', $_POST['field_Alarms_Execute_6']);

    SqliteDBSetValue($dbID, 'ALARMS_7_TYPE', $_POST['field_Alarms_Type_7']);
    if ($_POST['field_Alarms_Max_7'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_7_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_7_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_7_THRESHOLD', $_POST['field_Alarms_Threshold_7']);
    SqliteDBSetValue($dbID, 'ALARMS_7_ABATEMENT', $_POST['field_Alarms_Abatement_7']);
    SqliteDBSetValue($dbID, 'ALARMS_7_EXECUTE', $_POST['field_Alarms_Execute_7']);

    SqliteDBSetValue($dbID, 'ALARMS_8_TYPE', $_POST['field_Alarms_Type_8']);
    if ($_POST['field_Alarms_Max_8'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_8_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_8_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_8_THRESHOLD', $_POST['field_Alarms_Threshold_8']);
    SqliteDBSetValue($dbID, 'ALARMS_8_ABATEMENT', $_POST['field_Alarms_Abatement_8']);
    SqliteDBSetValue($dbID, 'ALARMS_8_EXECUTE', $_POST['field_Alarms_Execute_8']);

    SqliteDBSetValue($dbID, 'ALARMS_9_TYPE', $_POST['field_Alarms_Type_9']);
    if ($_POST['field_Alarms_Max_9'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_9_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_9_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_9_THRESHOLD', $_POST['field_Alarms_Threshold_9']);
    SqliteDBSetValue($dbID, 'ALARMS_9_ABATEMENT', $_POST['field_Alarms_Abatement_9']);
    SqliteDBSetValue($dbID, 'ALARMS_9_EXECUTE', $_POST['field_Alarms_Execute_9']);

    SqliteDBSetValue($dbID, 'ALARMS_10_TYPE', $_POST['field_Alarms_Type_10']);
    if ($_POST['field_Alarms_Max_10'][0] == "yes")
        SqliteDBSetValue($dbID, 'ALARMS_10_MAX', 'yes');
    else
        SqliteDBSetValue($dbID, 'ALARMS_10_MAX', 'no');
    SqliteDBSetValue($dbID, 'ALARMS_10_THRESHOLD', $_POST['field_Alarms_Threshold_10']);
    SqliteDBSetValue($dbID, 'ALARMS_10_ABATEMENT', $_POST['field_Alarms_Abatement_10']);
    SqliteDBSetValue($dbID, 'ALARMS_10_EXECUTE', $_POST['field_Alarms_Execute_10']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'alarms.php');

?>