<?php
//
//  File:       wview_upgrade.php
//
//  Purpose:    Upgrade to the latest version.
//

    require("functions.php");

    $pidstr = GetBinaryPrefix() . "/bin/wviewPlug-updater";
    if (file_exists($pidstr))
    {
        if (IswviewRunning() && IswviewIndicated())
        {
            // Stop wview:
            $systemstr = "sudo /etc/init.d/wview stop > /dev/null";
            system($systemstr);
            while (IswviewRunning() || IswviewIndicated())
            {
                sleep(1);
            }
        }

        $systemstr = "sudo" . $pidstr . "> /dev/null";
        system($systemstr);
        $latest_version = file_get_contents('http://www.wviewweather.com/wview-latest.txt');
        $current_version = wviewVersionGet();
        while ($current_version != $latest_version)
        {
            sleep(1);
            $current_version = wviewVersionGet();
        }

        header('Location: ' . 'system_status.php');
    }
?>
