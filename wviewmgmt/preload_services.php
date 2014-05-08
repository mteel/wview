<?php
//
//  File:       preload_services.php
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

    // Load all config items from the database:

    // Services:
    $field_Enable_htmlgend                  = SqliteDBGetValue($dbID, 'ENABLE_HTMLGEN');
    $field_Enable_wvalarmd                  = SqliteDBGetValue($dbID, 'ENABLE_ALARMS');
    $field_Enable_wvcwopd                   = SqliteDBGetValue($dbID, 'ENABLE_CWOP');
    $field_Enable_wvhttpd                   = SqliteDBGetValue($dbID, 'ENABLE_HTTP');
    if (SqliteDBGetValue($dbID, 'ENABLE_FTP') == "yes")
    {
        $field_Export_Remote_Type = "FTP";
    }
    else if (SqliteDBGetValue($dbID, 'ENABLE_SSH') == "yes")
    {
        $field_Export_Remote_Type = "SSH";
    }
    else
    {
        $field_Export_Remote_Type = "None";
    }
    $field_Enable_wvpmond                   = SqliteDBGetValue($dbID, 'ENABLE_PROCMON');

    $field_wviewd_Verbose = "no";
    $field_htmlgend_Verbose = "no";
    $field_wvalarmd_Verbose = "no";
    $field_wviewftpd_Verbose = "no";
    $field_wviewsshd_Verbose = "no";
    $field_wvcwopd_Verbose = "no";
    $field_wvhttpd_Verbose = "no";
    $verbosityMask = bindec(SqliteDBGetValue($dbID, 'STATION_VERBOSE_MSGS'));
    if (($verbosityMask & 0x01) != 0)
        $field_wviewd_Verbose = "yes";
    if (($verbosityMask & 0x02) != 0)
        $field_htmlgend_Verbose = "yes";
    if (($verbosityMask & 0x04) != 0)
        $field_wvalarmd_Verbose = "yes";
    if (($verbosityMask & 0x08) != 0)
        $field_wviewftpd_Verbose = "yes";
    if (($verbosityMask & 0x10) != 0)
        $field_wviewsshd_Verbose = "yes";
    if (($verbosityMask & 0x20) != 0)
        $field_wvcwopd_Verbose = "yes";
    if (($verbosityMask & 0x40) != 0)
        $field_wvhttpd_Verbose = "yes";

    $field_Enable_Email                     = SqliteDBGetValue($dbID, 'ENABLE_EMAIL_ALERTS');
    $field_Email_Address                    = SqliteDBGetValue($dbID, 'EMAIL_ADDRESS');
    $field_Email_From_Address               = SqliteDBGetValue($dbID, 'FROM_EMAIL_ADDRESS');
    $field_Send_Test_Email                  = SqliteDBGetValue($dbID, 'SEND_TEST_EMAIL');

    // ProcMon:
    $field_ProcMon_wviewd                   = SqliteDBGetValue($dbID, 'PROCMON_wviewd');
    $field_ProcMon_htmlgend                 = SqliteDBGetValue($dbID, 'PROCMON_htmlgend');
    $field_ProcMon_wvalarmd                 = SqliteDBGetValue($dbID, 'PROCMON_wvalarmd');
    $field_ProcMon_wvcwopd                  = SqliteDBGetValue($dbID, 'PROCMON_wvcwopd');
    $field_ProcMon_wvhttpd                  = SqliteDBGetValue($dbID, 'PROCMON_wvhttpd');

    // Close the database connection:
    SqliteDBClose($dbID);

?>

