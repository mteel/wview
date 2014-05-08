<?php
//
//  File:       process_ftp.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");


    // Open the database:
    $dbID = SqliteDBOpen();

    // FTP:
    SqliteDBSetValue($dbID, 'FTP_HOST', $_POST['field_FTP_Hostname']);
    SqliteDBSetValue($dbID, 'FTP_USERNAME', $_POST['field_FTP_Username']);
    SqliteDBSetValue($dbID, 'FTP_PASSWD', $_POST['field_FTP_Password']);
    SqliteDBSetValue($dbID, 'FTP_REMOTE_DIRECTORY', $_POST['field_FTP_Remote_Dir']);
    if ($_POST['field_FTP_Use_Passive'][0] == "yes")
        SqliteDBSetValue($dbID, 'FTP_USE_PASSIVE', 'yes');
    else
        SqliteDBSetValue($dbID, 'FTP_USE_PASSIVE', 'no');
    SqliteDBSetValue($dbID, 'FTP_INTERVAL', $_POST['field_FTP_Interval']);

    SqliteDBSetValue($dbID, 'FTP_RULE_1_SOURCE', $_POST['field_FTP_Source_1']);
    SqliteDBSetValue($dbID, 'FTP_RULE_2_SOURCE', $_POST['field_FTP_Source_2']);
    SqliteDBSetValue($dbID, 'FTP_RULE_3_SOURCE', $_POST['field_FTP_Source_3']);
    SqliteDBSetValue($dbID, 'FTP_RULE_4_SOURCE', $_POST['field_FTP_Source_4']);
    SqliteDBSetValue($dbID, 'FTP_RULE_5_SOURCE', $_POST['field_FTP_Source_5']);
    SqliteDBSetValue($dbID, 'FTP_RULE_6_SOURCE', $_POST['field_FTP_Source_6']);
    SqliteDBSetValue($dbID, 'FTP_RULE_7_SOURCE', $_POST['field_FTP_Source_7']);
    SqliteDBSetValue($dbID, 'FTP_RULE_8_SOURCE', $_POST['field_FTP_Source_8']);
    SqliteDBSetValue($dbID, 'FTP_RULE_9_SOURCE', $_POST['field_FTP_Source_9']);
    SqliteDBSetValue($dbID, 'FTP_RULE_10_SOURCE', $_POST['field_FTP_Source_10']);
    
    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'ftp.php');

?>