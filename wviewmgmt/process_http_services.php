<?php
//
//  File:       process_http_services.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // HTTP:
    SqliteDBSetValue($dbID, 'HTTP_WUSTATIONID', $_POST['field_HTTP_Wunderground_ID']);
    SqliteDBSetValue($dbID, 'HTTP_WUPASSWD', $_POST['field_HTTP_Wunderground_Password']);
    SqliteDBSetValue($dbID, 'HTTP_YOUSTATIONID', $_POST['field_HTTP_Weatherforyou_ID']);
    SqliteDBSetValue($dbID, 'HTTP_YOUPASSWD', $_POST['field_HTTP_Weatherforyou_Password']);
    

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'http_services.php');

?>