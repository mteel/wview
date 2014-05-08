<?php
//
//  File:       process_sql_export.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // SQL Export:
    SqliteDBSetValue($dbID, 'STATION_SQLDB_HOST', $_POST['field_SQL_Hostname']);
    SqliteDBSetValue($dbID, 'STATION_SQLDB_USERNAME', $_POST['field_SQL_Username']);
    SqliteDBSetValue($dbID, 'STATION_SQLDB_PASSWORD', $_POST['field_SQL_Password']);
    SqliteDBSetValue($dbID, 'STATION_SQLDB_DB_NAME', $_POST['field_SQL_Database']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'sql_export.php');

?>