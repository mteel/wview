<?php
//
//  File:       network_update.php
//
//  Purpose:    Upgrade network settings.
//

    require("functions.php");

    $interfaces = "/home/www-data/interfaces";
    $pfile = fopen($interfaces, "w");
    if ($pfile == false)
    {
        echo "$interfaces failed to open";
        sleep(2);
        header('Location: ' . 'system_status.php');
    }
    else
    {
        fputs($pfile, "auto lo\n");
        fputs($pfile, "iface lo inet loopback\n\n");

        fputs($pfile, "auto eth0\n");
        if ($_POST['field_network_dhcp'][0] == "yes")
        {
            fputs($pfile, "iface eth0 inet dhcp\n");
        }
        else
        {
            fputs($pfile, "iface eth0 inet static\n");
            fputs($pfile, "address " . $_POST['field_network_ip_adrs'] . "\n");
            fputs($pfile, "netmask " . $_POST['field_network_mask'] . "\n");
            fputs($pfile, "gateway " . $_POST['field_network_gw'] . "\n");

            $nsfile = "/home/www-data/resolv.conf";
            $namefile = fopen($nsfile, "w");
            fputs($namefile, "nameserver " . $_POST['field_network_dns1'] . "\n");
            fputs($namefile, "nameserver " . $_POST['field_network_dns2'] . "\n");
            fclose($namefile);
        }

        fclose($pfile);

        $syscmnd = "sudo /etc/init.d/networking restart > /dev/null";
        system($syscmnd);

        header('Location: ' . 'system_status.php');
    }
?>
