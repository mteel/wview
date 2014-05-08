<?php
//
//  File:       preload_system_status.php
//
//  Purpose:    Preload config items from the SQLite3 database and other places.
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


    // First determine if wview is running:
    $field_wview_is_running                 = IswviewRunning();
    $field_wview_is_indicated               = IswviewIndicated();

    // Get the latest version:
    $field_latest_version = file_get_contents('http://www.wviewweather.com/wview-latest.txt');

    // See if the wviewPlug updater is installed:
    $pidstr = "/usr/local/bin/wviewPlug-updater";
    if (file_exists($pidstr))
    {
        $field_updater_exists               = true;
    }
    else
    {
        $field_updater_exists               = false;
    }

    // Load network parameters:
    $pidstr = "/etc/network/interfaces";
    if (file_exists($pidstr))
    {
        $field_network_debian               = true;
    }
    else
    {
        $field_network_debian               = false;
    }

    if ($field_network_debian == false || NetworkTypeGet() == "dhcp")
    {
        $field_network_dhcp                 = "yes";
    }
    else
    {
        $field_network_dhcp                 = "no";
        $field_network_ip_adrs              = NetworkIPGet();
        $field_network_mask                 = NetworkMaskGet();
        $field_network_gw                   = NetworkGWGet();
        $field_network_dns1                 = NetworkDNS1Get();
        $field_network_dns2                 = NetworkDNS2Get();
    }

    // Close the database connection:
    SqliteDBClose($dbID);

?>
