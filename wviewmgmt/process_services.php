<?php
//
//  File:       process_services.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");

    // Open the database:
    $dbID = SqliteDBOpen();

    // Save all config items to the database:

    // Services:
    if ($_POST['field_Enable_htmlgend'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_HTMLGEN', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_HTMLGEN', 'no');
    if ($_POST['field_Enable_wvalarmd'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_ALARMS', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_ALARMS', 'no');
    if ($_POST['field_Enable_wvcwopd'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_CWOP', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_CWOP', 'no');
    if ($_POST['field_Enable_wvhttpd'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_HTTP', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_HTTP', 'no');

    if ($_POST['field_Export_Remote_Type'] == "FTP")
    {
        SqliteDBSetValue($dbID, 'ENABLE_FTP', 'yes');
        SqliteDBSetValue($dbID, 'ENABLE_SSH', 'no');
    }
    else if ($_POST['field_Export_Remote_Type'] == "SSH")
    {
        SqliteDBSetValue($dbID, 'ENABLE_FTP', 'no');
        SqliteDBSetValue($dbID, 'ENABLE_SSH', 'yes');
    }
    else
    {
        SqliteDBSetValue($dbID, 'ENABLE_FTP', 'no');
        SqliteDBSetValue($dbID, 'ENABLE_SSH', 'no');
    }

    if ($_POST['field_Enable_wvpmond'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_PROCMON', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_PROCMON', 'no');

    $verbosityMask = 0;
    if ($_POST['field_wviewd_Verbose'][0] == "yes")
        $verbosityMask |= 0x01;
    if ($_POST['field_htmlgend_Verbose'][0] == "yes")
        $verbosityMask |= 0x02;
    if ($_POST['field_wvalarmd_Verbose'][0] == "yes")
        $verbosityMask |= 0x04;
    if ($_POST['field_wviewftpd_Verbose'][0] == "yes")
        $verbosityMask |= 0x08;
    if ($_POST['field_wviewsshd_Verbose'][0] == "yes")
        $verbosityMask |= 0x10;
    if ($_POST['field_wvcwopd_Verbose'][0] == "yes")
        $verbosityMask |= 0x20;
    if ($_POST['field_wvhttpd_Verbose'][0] == "yes")
        $verbosityMask |= 0x40;
    $verbosityStr = sprintf("%08b", $verbosityMask);
    SqliteDBSetValue($dbID, 'STATION_VERBOSE_MSGS', $verbosityStr);

    if ($_POST['field_Enable_Email'][0] == "yes")
        SqliteDBSetValue($dbID, 'ENABLE_EMAIL_ALERTS', 'yes');
    else
        SqliteDBSetValue($dbID, 'ENABLE_EMAIL_ALERTS', 'no');
    SqliteDBSetValue($dbID, 'EMAIL_ADDRESS', $_POST['field_Email_Address']);
    SqliteDBSetValue($dbID, 'FROM_EMAIL_ADDRESS', $_POST['field_Email_From_Address']);
    if ($_POST['field_Send_Test_Email'][0] == "yes")
        SqliteDBSetValue($dbID, 'SEND_TEST_EMAIL', 'yes');
    else
        SqliteDBSetValue($dbID, 'SEND_TEST_EMAIL', 'no');

    // ProcMon:
    SqliteDBSetValue($dbID, 'PROCMON_wviewd', $_POST['field_ProcMon_wviewd']);
    SqliteDBSetValue($dbID, 'PROCMON_htmlgend', $_POST['field_ProcMon_htmlgend']);
    SqliteDBSetValue($dbID, 'PROCMON_wvalarmd', $_POST['field_ProcMon_wvalarmd']);
    SqliteDBSetValue($dbID, 'PROCMON_wvcwopd', $_POST['field_ProcMon_wvcwopd']);
    SqliteDBSetValue($dbID, 'PROCMON_wvhttpd', $_POST['field_ProcMon_wvhttpd']);

    // Close the database connection:
    SqliteDBClose($dbID);

    header('Location: ' . 'services.php');
?>