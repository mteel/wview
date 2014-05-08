<?php
//
//  File:       process_cwop.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // Save all config items to the database:

    // CWOP:
    SqliteDBSetValue($dbID, 'CWOP_APRS_CALL_SIGN', $_POST['field_CWOP_CallSign']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_SERVER1', $_POST['field_CWOP_Server_1']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_PORTNO1', $_POST['field_CWOP_Port_1']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_SERVER2', $_POST['field_CWOP_Server_2']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_PORTNO2', $_POST['field_CWOP_Port_2']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_SERVER3', $_POST['field_CWOP_Server_3']);
    SqliteDBSetValue($dbID, 'CWOP_APRS_PORTNO3', $_POST['field_CWOP_Port_3']);
    SqliteDBSetValue($dbID, 'CWOP_LATITUDE', $_POST['field_CWOP_Latitude']);
    SqliteDBSetValue($dbID, 'CWOP_LONGITUDE', $_POST['field_CWOP_Longitude']);
    if ($_POST['field_CWOP_Log_Packet'][0] == "yes")
        SqliteDBSetValue($dbID, 'CWOP_LOG_WX_PACKET', 'yes');
    else
        SqliteDBSetValue($dbID, 'CWOP_LOG_WX_PACKET', 'no');


    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'cwop.php');

?>