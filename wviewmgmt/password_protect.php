<?php

require("functions.php");

################################################################################
# Add following HTML code to your page where you want to have logout link
# <a href="http://www.example.com/path/to/protected/page.php?logout=1">Logout</a>
################################################################################

// User will be redirected to this page after logout
define('LOGOUT_URL', 'logout.php');

// time out after NN minutes of inactivity. Set to 0 to not timeout
define('TIMEOUT_MINUTES', 30);

// This parameter is only useful when TIMEOUT_MINUTES is not zero
// true - timeout time from last activity, false - timeout time from login
define('TIMEOUT_CHECK_ACTIVITY', true);


function getDBPassword()
{
    // Open the database:
    $dbID = SqliteDBOpen();

    // Get MD5 password:
    $retVal = SqliteDBGetValue($dbID, 'ADMIN_PASSWORD');
    if ($retVal == false)
    {
        // Insert the default:
        $passwd = md5("wview");
        SqliteDBCreateValue($dbID, 'ADMIN_PASSWORD', $passwd, 'Admin Password (md5):', 'NULL');
        $retVal = $passwd;
    }

    SqliteDBClose($dbID);
    return $retVal;
}

// timeout in seconds
$timeout = (TIMEOUT_MINUTES == 0 ? 0 : time() + TIMEOUT_MINUTES * 60);

// logout?
if(isset($_GET['logout']))
{
    setcookie("verify", '', $timeout, '/'); // clear password;
    header('Location: ' . LOGOUT_URL);
    exit();
}

if(!function_exists('showLoginPasswordProtect'))
{
    // show login form
    function showLoginPasswordProtect($error_msg)
    {
?>
<html>
<head>
  <title>Please enter the admin password:</title>
  <META HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE">
  <META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">
  <script type="text/javascript">
    var formInUse = false;
    function setFocus()
    {
      if(!formInUse)
      {
        document.SubmitForm.access_password.focus();
      }
    }
  </script>
</head>
<body onload="setFocus()">
  <style>
    input { border: 1px solid black; }
  </style>
  <div style="width:500px; margin-left:auto; margin-right:auto; text-align:center">
  <form method="post" name="SubmitForm">
    <table border="4" width="480" height="180">
      <tr align="center">
        <td>
          <h3>Please enter the admin password:</h3>
          <font color="red"><?php echo $error_msg;?></font>
          <input type="password" name="access_password" formInUse="true" />
          <p></p>
          <input type="submit" name="Submit" value="Submit" />
          <p></p>
        </td>
      </tr>
    </table>
  </form>
  <br>
  </div>
</body>
</html>
<?php
    // stop at this point
    die();
    }
}

// user provided password
if (isset($_POST['access_password']))
{

    $login = isset($_POST['access_login']) ? $_POST['access_login'] : '';
    $pass = $_POST['access_password'];
    $md5pass = md5($pass);
    $dbpasswd = getDBPassword();
    if ($dbpasswd != $md5pass)
    {
        showLoginPasswordProtect("Incorrect password.");
    }
    else
    {
        // set cookie if password was validated
        setcookie("verify", $dbpasswd, $timeout, '/');

        unset($_POST['access_login']);
        unset($_POST['access_password']);
        unset($_POST['Submit']);
    }
}
else
{
    // check if password cookie is set
    if (!isset($_COOKIE['verify']))
    {
        showLoginPasswordProtect("");
    }

    // check if cookie is good
    $dbpasswd = getDBPassword();
    if ($_COOKIE['verify'] == $dbpasswd)
    {
        // prolong timeout
        if (TIMEOUT_CHECK_ACTIVITY)
        {
            setcookie("verify", $dbpasswd, $timeout, '/');
        }
    }
    else
    {
        showLoginPasswordProtect("");
    }
}

?>
