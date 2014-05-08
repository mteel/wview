<?php
//
//  File:       password_update.php
//
//  Purpose:    Submit config item changes to the SQLite3 database.
//

    require("functions.php");

    // Admin Password:
    if ($_POST['field_Admin_Password1'] != "")
    {
        // Compare both versions:
        if ($_POST['field_Admin_Password1'] == $_POST['field_Admin_Password2'])
        {
            // Change the password:
            $newpassword = $_POST['field_Admin_Password1'];
            $newpassword = md5($newpassword);
            $dbID = SqliteDBOpen();
            SqliteDBSetValue($dbID, 'ADMIN_PASSWORD', $newpassword);
            SqliteDBClose($dbID);
            header('Location: ' . 'system_status.php');
        }
        else
        {
            // Password mismatch:
            header('Location: ' . 'system_status.php?mismatch');
        }
    }
?>