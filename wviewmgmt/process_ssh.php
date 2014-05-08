<?php
//
//  File:       process_ssh.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // SSH:
    SqliteDBSetValue($dbID, 'SSH_1_SOURCE', $_POST['field_SSH_Source_1']);
    SqliteDBSetValue($dbID, 'SSH_1_INTERVAL', $_POST['field_SSH_Interval_1']);
    SqliteDBSetValue($dbID, 'SSH_1_HOST', $_POST['field_SSH_Host_1']);
    SqliteDBSetValue($dbID, 'SSH_1_PORT', $_POST['field_SSH_Port_1']);
    SqliteDBSetValue($dbID, 'SSH_1_USERNAME', $_POST['field_SSH_UserName_1']);
    SqliteDBSetValue($dbID, 'SSH_1_DESTINATION', $_POST['field_SSH_Destination_1']);
    SqliteDBSetValue($dbID, 'SSH_1_TIMEOUT', $_POST['field_SSH_Timeout_1']);
    SqliteDBSetValue($dbID, 'SSH_2_SOURCE', $_POST['field_SSH_Source_2']);
    SqliteDBSetValue($dbID, 'SSH_2_INTERVAL', $_POST['field_SSH_Interval_2']);
    SqliteDBSetValue($dbID, 'SSH_2_HOST', $_POST['field_SSH_Host_2']);
    SqliteDBSetValue($dbID, 'SSH_2_PORT', $_POST['field_SSH_Port_2']);
    SqliteDBSetValue($dbID, 'SSH_2_USERNAME', $_POST['field_SSH_UserName_2']);
    SqliteDBSetValue($dbID, 'SSH_2_DESTINATION', $_POST['field_SSH_Destination_2']);
    SqliteDBSetValue($dbID, 'SSH_2_TIMEOUT', $_POST['field_SSH_Timeout_2']);
    SqliteDBSetValue($dbID, 'SSH_3_SOURCE', $_POST['field_SSH_Source_3']);
    SqliteDBSetValue($dbID, 'SSH_3_INTERVAL', $_POST['field_SSH_Interval_3']);
    SqliteDBSetValue($dbID, 'SSH_3_HOST', $_POST['field_SSH_Host_3']);
    SqliteDBSetValue($dbID, 'SSH_3_PORT', $_POST['field_SSH_Port_3']);
    SqliteDBSetValue($dbID, 'SSH_3_USERNAME', $_POST['field_SSH_UserName_3']);
    SqliteDBSetValue($dbID, 'SSH_3_DESTINATION', $_POST['field_SSH_Destination_3']);
    SqliteDBSetValue($dbID, 'SSH_3_TIMEOUT', $_POST['field_SSH_Timeout_3']);
    SqliteDBSetValue($dbID, 'SSH_4_SOURCE', $_POST['field_SSH_Source_4']);
    SqliteDBSetValue($dbID, 'SSH_4_INTERVAL', $_POST['field_SSH_Interval_4']);
    SqliteDBSetValue($dbID, 'SSH_4_HOST', $_POST['field_SSH_Host_4']);
    SqliteDBSetValue($dbID, 'SSH_4_PORT', $_POST['field_SSH_Port_4']);
    SqliteDBSetValue($dbID, 'SSH_4_USERNAME', $_POST['field_SSH_UserName_4']);
    SqliteDBSetValue($dbID, 'SSH_4_DESTINATION', $_POST['field_SSH_Destination_4']);
    SqliteDBSetValue($dbID, 'SSH_4_TIMEOUT', $_POST['field_SSH_Timeout_4']);
    SqliteDBSetValue($dbID, 'SSH_5_SOURCE', $_POST['field_SSH_Source_5']);
    SqliteDBSetValue($dbID, 'SSH_5_INTERVAL', $_POST['field_SSH_Interval_5']);
    SqliteDBSetValue($dbID, 'SSH_5_HOST', $_POST['field_SSH_Host_5']);
    SqliteDBSetValue($dbID, 'SSH_5_PORT', $_POST['field_SSH_Port_5']);
    SqliteDBSetValue($dbID, 'SSH_5_USERNAME', $_POST['field_SSH_UserName_5']);
    SqliteDBSetValue($dbID, 'SSH_5_DESTINATION', $_POST['field_SSH_Destination_5']);
    SqliteDBSetValue($dbID, 'SSH_5_TIMEOUT', $_POST['field_SSH_Timeout_5']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'ssh.php');
?>
