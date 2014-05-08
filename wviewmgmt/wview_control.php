<?php
//
//  File:       wview_control.php
//
//  Purpose:    Submit status changes.
//

    require("functions.php");

    if (IswviewRunning() && IswviewIndicated())
    {
        // Stop wview:
        $systemstr = "sudo /etc/init.d/wview stop > /dev/null";
        system($systemstr);
    }
    else
    {
        if (! IswviewRunning())
        {
            // Start wview:
            $systemstr = "sudo /etc/init.d/wview start > /dev/null";
            system($systemstr);
            sleep(1);
        }
    }

    header('Location: ' . 'system_status.php');
?>
