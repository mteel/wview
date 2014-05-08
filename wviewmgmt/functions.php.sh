$NONE = '"display: none"';
$EMPTY = '"display: "';

// Process status:
function IsStatusAvailable($procname)
{
    $pidstr = "/var/run/wview/" . $procname . ".sts";
    return (file_exists($pidstr));
}

function DisplayStatusColor($status)
{
    switch($status)
    {
        case 0:
            return "<img src=imgs/black.png />";
        case 1:
            return "<img src=imgs/blue.png />";
        case 2:
            return "<img src=imgs/yellow.png />";
        case 3:
            return "<img src=imgs/green.png />";
        case 4:
            return "<img src=imgs/gray.png />";
        case 5:
            return "<img src=imgs/red.png />";
    }
}

function DisplayStatuses()
{
    $procs          = array('wview', 'html', 'alarms', 'cwop', 'http', 'ftp', 'ssh', 'pmon');
    $statusLabels   = array("Not Started", 
                            "Booting", 
                            "Wait for wviewd", 
                            "Running", 
                            "Shutdown", 
                            "Error"); 

    echo "<table border=\"1\" width=\"100%\">";
    echo "  <tr>";
    echo "    <th width=\"5%\">Service</th>";
    echo "    <th colspan=2 width=\"11%\">Status</th>";
    echo "    <th width=\"12%\">Message</th>";
    echo "    <th width=\"10%\">Stat1</th>";
    echo "    <th width=\"8%\">Value</th>";
    echo "    <th width=\"10%\">Stat2</th>";
    echo "    <th width=\"8%\">Value</th>";
    echo "    <th width=\"10%\">Stat3</th>";
    echo "    <th width=\"8%\">Value</th>";
    echo "    <th width=\"10%\">Stat4</th>";
    echo "    <th width=\"8%\">Value</th>";
    echo "  </tr>";
    
    foreach ($procs as $process)
    {
        echo "<tr>";
        echo "<td>$process</td>";
        if (IsStatusAvailable($process))
        {
            $pidstr = "/var/run/wview/" . $process . ".sts";
            $ini_array = parse_ini_file($pidstr);
            echo "<td>" . DisplayStatusColor($ini_array['status']) . "</td>";
            echo "<td>" . $statusLabels[$ini_array['status']] . "</td>";
            if (array_key_exists('message', $ini_array))
            {
                echo "<td>" . $ini_array['message'] . "</td>";
            }
            else
            {
                echo "<td>&nbsp;</td>";
            }
            if (array_key_exists('desc0', $ini_array))
            {
                echo "<td>" . $ini_array['desc0'] . "</td>";
                echo "<td>" . $ini_array['stat0'] . "</td>";
            }
            else
            {
                echo "<td>&nbsp;</td>";
                echo "<td>&nbsp;</td>";
            }
            if (array_key_exists('desc1', $ini_array))
            {
                echo "<td>" . $ini_array['desc1'] . "</td>";
                echo "<td>" . $ini_array['stat1'] . "</td>";
            }
            else
            {
                echo "<td>&nbsp;</td>";
                echo "<td>&nbsp;</td>";
            }
            if (array_key_exists('desc2', $ini_array))
            {
                echo "<td>" . $ini_array['desc2'] . "</td>";
                echo "<td>" . $ini_array['stat2'] . "</td>";
            }
            else
            {
                echo "<td>&nbsp;</td>";
                echo "<td>&nbsp;</td>";
            }
            if (array_key_exists('desc3', $ini_array))
            {
                echo "<td>" . $ini_array['desc3'] . "</td>";
                echo "<td>" . $ini_array['stat3'] . "</td>";
            }
            else
            {
                echo "<td>&nbsp;</td>";
                echo "<td>&nbsp;</td>";
            }
        }
        else
        {
            echo "<td>" . DisplayStatusColor(0) . "</td>";
            echo "<td>" . $statusLabels[0] . "</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
            echo "<td>&nbsp;</td>";
        }
        echo "</tr>";
    }
    
    echo "</table>";
}


// Network setup:
function NetworkTypeGet()
{
    $lines = file('/etc/network/interfaces', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stripos($lines[$i], 'iface eth0');
        if ($pos !== false)
        {
            $type = stripos($lines[$i], 'dhcp');
            if ($type !== false)
            {
                return 'dhcp';
            }
            $type = stripos($lines[$i], 'static');
            if ($type !== false)
            {
                return 'static';
            }
            else
            {
                return 'none';
            }
        }
    }

    // If here, we found nothing:
    return 'none';
}

function NetworkIPGet()
{
    $lines = file('/etc/network/interfaces', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stristr($lines[$i], 'address');
        if ($pos !== false)
        {
            $address = substr($pos, 7);
            $address = trim($address);
            return $address;
        }
    }

    // If here, we found nothing:
    return 'none';
}

function NetworkMaskGet()
{
    $lines = file('/etc/network/interfaces', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stristr($lines[$i], 'netmask');
        if ($pos !== false)
        {
            $address = substr($pos, 7);
            $address = trim($address);
            return $address;
        }
    }

    // If here, we found nothing:
    return 'none';
}

function NetworkGWGet()
{
    $lines = file('/etc/network/interfaces', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stristr($lines[$i], 'gateway');
        if ($pos !== false)
        {
            $address = substr($pos, 7);
            $address = trim($address);
            return $address;
        }
    }

    // If here, we found nothing:
    return 'none';
}

function NetworkDNS1Get()
{
    $lines = file('/etc/resolv.conf', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stristr($lines[$i], 'nameserver');
        if ($pos !== false)
        {
            $address = substr($pos, 10);
            $address = trim($address);
            return $address;
        }
    }

    // If here, we found nothing:
    return 'none';
}

function NetworkDNS2Get()
{
    $lines = file('/etc/resolv.conf', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    $found = false;

    for ($i = 0; $i < sizeof($lines); $i++)
    {
        $pos = stristr($lines[$i], 'nameserver');
        if ($pos !== false)
        {
            if ($found == false)
            {
                $found = true;
            }
            else
            {
                $address = substr($pos, 10);
                $address = trim($address);
                return $address;
            }
        }
    }

    // If here, we found nothing:
    return 'none';
}

function SqliteDBOpen()
{
    $connectStr = GetConfigPrefix() . "/wview/wview-conf.sdb";
    try
    {
        $conn = new PDO('sqlite:'.$connectStr);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBOpen " . $connectStr . " failed: " . $exception->getMessage());
    }

    return $conn;
}

function SqliteDBClose($dbConn)
{
    $dbConn = NULL;
}

function SqliteDBCreateValue($dbID, $name, $value, $descr, $dependsOn)
{
    $sql = false;

    if ($dependsOn == 'NULL')
        $sql = "INSERT INTO config VALUES('" . $name . "','" . $value . "','" . $descr . "','')";
    else
        $sql = "INSERT INTO config VALUES('" . $name . "','" . $value . "','" . $descr . "','" . $dependsOn . "')";
    try
    {
        $result = $dbID->exec($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBCreateValue (" . $name . "," . $value . ") failed: " . $exception->getMessage());
    }

    if ($result != 1)
        die("SqliteDBCreateValue: No rows updated: is the wview-conf.sdb file and containing directory writable by the http user?");
}

function SqliteDBSetValue($dbID, $name, $value)
{
    $sql = "update config set value='" .$value ."' where name='" . $name . "'";
    try
    {
        $result = $dbID->exec($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBSetValue (" . $name . "," . $value . ") failed: " . $exception->getMessage());
    }

    if ($result != 1)
    {
        $errmsg = "SqliteDBSetValue: No rows updated for parameter $name: ";
        $errmsg .= "is the wview-conf.sdb database and containing directory writable by the http user -OR- ";
        $errmsg .= "is your configuration database from an older version of wview and missing the $name row?";
        die($errmsg);
    }
}

function SqliteDBGetValue($dbID, $name)
{
    $sql = "SELECT value FROM config WHERE name='" . $name . "'";
    try
    {
        $result = $dbID->query($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBGetValue (" . $name . ") failed: " . $exception->getMessage());
    }

    if ($result == NULL)
    {
        return false;
    }

    $retVal = $result->fetchColumn();
    $result->closeCursor();

    return $retVal;
}

function SqliteDBGetRowCount($dbID)
{
    $sql = "SELECT COUNT(*) FROM config";
    try
    {
        $result = $dbID->query($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBGetRowCount failed: " . $exception->getMessage());
    }

    $retVal = $result->fetchColumn();
    $result->closeCursor();

    return $retVal;
}

function SqliteDBArchiveGetRowCount()
{
    $retVal = 0;

    $connectStr = GetDataPrefix() . "/wview/archive/wview-archive.sdb";
    try
    {
        $conn = new PDO('sqlite:'.$connectStr);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBOpen " . $connectStr . " failed: " . $exception->getMessage());
    }

    $sql = "SELECT COUNT(*) FROM archive";
    try
    {
        $result = $conn->query($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBGetRowCount failed: " . $exception->getMessage());
    }

    $retVal = $result->fetchColumn();
    $result->closeCursor();
    $dbConn = NULL;
    return $retVal;
}

function SqliteDBArchiveGetDateRange()
{
    $retVal = 0;

    $connectStr = GetDataPrefix() . "/wview/archive/wview-archive.sdb";
    try
    {
        $conn = new PDO('sqlite:'.$connectStr);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBOpen " . $connectStr . " failed: " . $exception->getMessage());
    }

    $sql = "SELECT MIN(dateTime) FROM archive";
    try
    {
        $result = $conn->query($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBGetRowCount failed: " . $exception->getMessage());
    }
    $start = $result->fetchColumn();
    $start = getdate($start);
    $result->closeCursor();

    $sql = "SELECT MAX(dateTime) FROM archive";
    try
    {
        $result = $conn->query($sql);
    }
    catch( PDOException $exception )
    {
        die("SqliteDBGetRowCount failed: " . $exception->getMessage());
    }
    $stop = $result->fetchColumn();
    $stop = getdate($stop);
    $result->closeCursor();
    $dbConn = NULL;

    $retVal = "$start[month] $start[mday], $start[year] - $stop[month] $stop[mday], $stop[year]";
    return $retVal;
}

function SqliteDBHiLowGetRowCount()
{
    $retVal = 0;

    $connectStr = GetDataPrefix() . "/wview/archive/wview-hilow.sdb";
    if (file_exists($connectStr))
    {
        try
        {
            $conn = new PDO('sqlite:'.$connectStr);
        }
        catch( PDOException $exception )
        {
            die("SqliteDBOpen " . $connectStr . " failed: " . $exception->getMessage());
        }
    
        $sql = "SELECT COUNT(*) FROM outTemp";
        try
        {
            $result = $conn->query($sql);
        }
        catch( PDOException $exception )
        {
            die("SqliteDBGetRowCount failed: " . $exception->getMessage());
        }
    
        $retVal = $result->fetchColumn();
        $result->closeCursor();
        $dbConn = NULL;
    }

    return $retVal;
}

function SqliteDBNOAAGetRowCount()
{
    $retVal = 0;

    $connectStr = GetDataPrefix() . "/wview/archive/wview-noaa.sdb";
    if (file_exists($connectStr))
    {
        try
        {
            $conn = new PDO('sqlite:'.$connectStr);
        }
        catch( PDOException $exception )
        {
            die("SqliteDBOpen " . $connectStr . " failed: " . $exception->getMessage());
        }
    
        $sql = "SELECT COUNT(*) FROM noaaHistory";
        try
        {
            $result = $conn->query($sql);
        }
        catch( PDOException $exception )
        {
            die("SqliteDBGetRowCount failed: " . $exception->getMessage());
        }
    
        $retVal = $result->fetchColumn();
        $result->closeCursor();
        $dbConn = NULL;
    }

    return $retVal;
}

function IswviewRunning()
{
    $pidstr = GetDataPrefix() . "/wview/wviewd.pid";
    return (file_exists($pidstr));
}

function IswviewIndicated()
{
    $dbconn = SqliteDBOpen();
    $htmlEnabled = SqliteDBGetValue($dbconn, 'ENABLE_HTMLGEN');
    SqliteDBClose($dbconn);

    if ($htmlEnabled == "yes")
    {
        $pidstr = GetDataPrefix() . "/wview/wview-running";
        return (file_exists($pidstr));
    }
    else
    {
        return IswviewRunning();
    }
}

function wviewVersionGet()
{
    $pidstr = GetConfigPrefix() . "/wview/wview-version";
    return (file_get_contents($pidstr));
}

function IsStartupActive()
{
    if (IswviewRunning())
    {
        if (IswviewIndicated())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

?>
