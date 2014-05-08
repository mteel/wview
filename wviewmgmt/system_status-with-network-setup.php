<?php require("password_protect.php") ?>
<html>

<head>
  <meta name="Pragma" content="No-Cache">
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <meta http-equiv="refresh" content='
    <?php 
      if (IsStartupActive()) 
          echo "3"; 
      else 
          echo "60";
    ?>'
  >
  <link href="style.css" rel="stylesheet" type="text/css">
  <title>wview Management</title>
  <base target="_self">
  <style fprolloverstyle>
    A:hover {color: #00FFFF}
    .help:link {text-decoration: underline}
    .help:visited {text-decoration: underline}
    .help:hover {color: #FFCC00; text-decoration: underline}
    .logout:link {text-decoration: none}
    .logout:visited {text-decoration: none}
    .logout:hover {color: #FFCC00; text-decoration: none}
  </style>
  <style type="text/css">
    body {font-family: Arial, Verdana, sans-serif, Helvetica; background-color: #ffffff;}
    td, th, input, select {font-size: 11px}
  </style>

  <script name="JavaScript">
  <!--
  function reloadPage()
  {
      window.location = "system_status.php";
  }

  function show(el)
  { 
      obj = document.getElementById(el).style; 
      if (obj.display == 'none')
      {
        obj.display = ''; 
      }
  } 

  function hide(el)
  { 
      obj = document.getElementById(el).style; 
      if (obj.display == '')
      {
        obj.display = 'none'; 
      }
  } 

  function hideShowNetwork(status)
  {
     
     if (status == false)
     {
        hide('field_network_ip_adrs');
        hide('field_network_mask');
        hide('field_network_gw');
        hide('field_network_dns1');
        hide('field_network_dns2');
        hide('field_network_header_long');
        show('field_network_header_short');
     }
     else
     {
        show('field_network_ip_adrs');
        show('field_network_mask');
        show('field_network_gw');
        show('field_network_dns1');
        show('field_network_dns2');
        hide('field_network_header_short');
        show('field_network_header_long');
     }
  }

  function hideShowUpdateObj(enabled)
  {
     objWmr = document.getElementById('wviewupdate').style;
     if (enabled == false && objWmr.display == '')
     {
        hide('wviewupdate');
     }
     else if (enabled == true && objWmr.display == 'none')
     {
        show('wviewupdate');
     }
  }

  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_system_status.php") ?>

<DIV align=center>
<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber11" width="960" height="104">
  <tr>
    <td colspan="3" align="center" valign="center" width="960" height="100" bgcolor="#6666CC"> 
      <img border="0" src="wview-100x100.png">
    </td>
  </tr>

  <tr> 
    <td colspan="3" valign="top"> <img border="0" src="imgs/blueline.gif" width="960" height="11"></td>
  </tr>
</table>
   
<!-- Header -->
<table height=70 cellSpacing=0 cellPadding=0 width=960 bgColor=black border=0>

  <tr> 
    <!-- Page Name: -->
    <td width="130" height=70 rowspan="3" align=middle bordercolor="#000000" bgColor=black style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">System Status</font></H3></td>

    <!-- Station Name: -->
    <td width=700 height=33 align="center" vAlign=middle bordercolor="#000000" bgColor=#6666CC style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
      <p align="center"><b><font color="#FFFFFF"><span lang="en-us">
        <?php 
          if ($field_Station_Type == "Davis Vantage Pro") 
              echo "Davis Vantage Pro"; 
          else if ($field_Station_Type == "La Crosse WS-23XX")
              echo "La Crosse WS-23XX";
          else if ($field_Station_Type == "Oregon Scientific WMR9XX")
              echo "Oregon Scientific WMR9XX";
          else if ($field_Station_Type == "Viasala WXT510")
              echo "Viasala WXT510";
          else if ($field_Station_Type == "Texas Weather Instruments")
              echo "Texas Weather Instruments";
          else if ($field_Station_Type == "Virtual") 
              echo "Virtual";
          else if ($field_Station_Type == "Simulator") 
              echo "Simulator";
          else
              echo "[Bad Station Type In wview-conf.sdb Database]";
        ?>
        Weather Server and Site Generator</span></font></b>
    </td>

    <!-- wview Version: -->
    <td vAlign=center width=130 bgColor=#000000 style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black" bordercolor="#000000"> 
      <p align="center"><font color="#FFFFFF"> <span style="font-size: 8pt"><b>Version <?php echo $wview_version ?></b></span></font> 
    </td>
  </tr>

  <tr> 
    <td height=12 colspan="2" vAlign=center bordercolor="#000000" bgColor=#000000 style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black">
      <table width="830" border="0" cellspacing="0" cellpadding="0" >

        <!-- Column Layout: -->
        <tr  align="center"> 
          <td width="70" height="8" valign="middle" background="imgs/header-norm.gif" style=""></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
		  <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
          <td width="70" valign="middle" background="imgs/header-norm.gif"></td>
        </tr>

        <!-- Site Menu: -->
        <tr  align="center" valign="middle"> 
          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              System<br>Status</font></b> </td>

          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-top: 0; margin-bottom: 4"><b><a class="mainmenu" href="services.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Services</a></b> </td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-top: 0; margin-bottom: 4"><b><a class="mainmenu" href="station.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Station</a></b> </td>
          <td height="28" bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p align="center" style="margin-bottom: 4"> <b><a class="mainmenu" href="file_generation.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              File<br>Generation</a></b> </td>
		  <td bgcolor="#000000" style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
            <p style="margin-bottom: 4"><b><a class="mainmenu" href="alarms.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Alarms</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"><b> <a class="mainmenu" href="ftp.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              FTP</a></b> </td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b> <a class="mainmenu" href="ssh.php" style="font-size: 8pt; text-decoration: none; font-weight:700">
              SSH</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b> <a class="mainmenu" href="cwop.php" style="font-size: 8pt; text-decoration: none; font-weight:700">
              CWOP</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b> <a class="mainmenu" href="http_services.php" style="font-size: 8pt; text-decoration: none; font-weight:700">
              HTTP<br>Services</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b> <a class="mainmenu" href="calibration.php" style="font-size: 8pt; text-decoration: none; font-weight:700">
              Calibration</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b> <a class="mainmenu" href="sql_export.php" style="font-size: 8pt; text-decoration: none; font-weight:700">
              SQL<br>Export</a></b></td>
        </tr>
      </table>
    </td>
  </tr>
</table>

<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber9" width="960">
      
  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">System Status</font></td>
    <td width="6" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="500" valign="top" bgcolor="#FFFFFF"></td>
    <td width="180" valign="top" bgcolor="#FFFFFF"></td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan=<?php if ($field_updater_exists == true) echo "13"; else echo "12"; ?> valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>


  <!-- wview run status: -->
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      wview is:
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="red">
      <?php 
        if (IswviewRunning())
        {
            if (IswviewIndicated())
            {
                echo "Running";
            }
            else
            {
                echo "Starting";
            }
        }
        else
        {
            echo "Stopped";
        }
      ?>
      </font>
    </td>
    <td></td>
  </tr>
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF">
      <form method=post enctype=multipart/form-data action=wview_control.php>
        <input id="wviewcontrol" type="submit" value='<?php if ($field_wview_is_running) echo "Stop wview"; else echo "Start wview"; ?>' >
      </form>
    </td>
    <td></td>
  </tr>

  <!-- Space: -->
  <tr>    
    <td height="20" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td></td>
  </tr>

  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      The latest wview version is:
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="green">
      <?php echo $field_latest_version; ?>
      </font>
    </td>
    <td></td>
  </tr>
  <tr id="wviewupdate" style="display: <?php
                if ($field_updater_exists == true)
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF">
      <form method=post enctype=multipart/form-data action=wview_upgrade.php>
        <input id="wviewcontrol" type="submit" 
         value='Upgrade Now'>
      </form>
    </td>
    <td></td>
  </tr>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="20" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="top" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Archive Database Info: -->
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      Archive Database Records:
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="red">
      <?php echo SqliteDBArchiveGetRowCount(); ?>
      </font>
    </td>
    <td></td>
  </tr>
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      Archive Record Date Range:
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="red">
      <?php echo SqliteDBArchiveGetDateRange(); ?>
      </font>
    </td>
    <td></td>
  </tr>

  <!-- Space: -->
  <tr>    
    <td height="20" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td></td>
  </tr>

  <!-- Hilow Database Info: -->
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      HiLow Database Records:
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="red">
      <?php echo SqliteDBHiLowGetRowCount(); ?>
      </font>
    </td>
    <td></td>
  </tr>

  <!-- NOAA Database Info: -->
  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">
      NOAA Database Records (days):
      </label>
    </td>
    <td valign="top" bgcolor="#FFFFFF">
      <font color="red">
      <?php echo SqliteDBNOAAGetRowCount(); ?>
      </font>
    </td>
    <td></td>
  </tr>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="20" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="top" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Services</font></td>
    <td width="6" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="500" valign="top" bgcolor="#FFFFFF"></td>
    <td width="180" valign="top" bgcolor="#FFFFFF"></td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan=4 valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          wview services status. Statistics are since the last wview start.
        </font>
      </div>
    </td>
  </tr>

  <tr>
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td colspan=2 valign="top" bgcolor="#FFFFFF"> 
      <?php DisplayStatuses(); ?>
    </td>
    <td></td>
  </tr>

  <!-- Space: -->
  <tr>    
    <td height="20" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td></td>
  </tr>


  <!-- Horizontal rule: -->
  <tr> 
    <td height="20" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="top" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr id="field_network_header_long" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Network</font></td>
    <td width="6" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="500" valign="top" bgcolor="#FFFFFF"></td>
    <td width="180" valign="top" bgcolor="#FFFFFF"></td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="10" valign="top" bgcolor="#6666CC" align="center"> 
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          Configure network settings.
        </font>
        <br>
      </div>
    </td>
  </tr>

  <tr id="field_network_header_short" style="display: <?php if ($field_network_dhcp == "yes") echo ""; else echo "none"; ?>"> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Network</font></td>
    <td width="6" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="500" valign="top" bgcolor="#FFFFFF"></td>
    <td width="180" valign="top" bgcolor="#FFFFFF"></td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="5" valign="top" bgcolor="#6666CC" align="center"> 
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          Configure network settings.
        </font>
        <br>
      </div>
    </td>
  </tr>

  <form method=post enctype=multipart/form-data action=network_update.php>
  <!-- Checkbox: -->
  <tr id="wviewupdate" style="display: <?php
                if ($field_network_debian == true)
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable automatic network setup (DHCP)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_network_dhcp[] id=field_network_dhcp_option_1 value="yes" 
        <?php if ($field_network_dhcp == "yes") echo "checked"; ?> onclick="hideShowNetwork(!this.checked)">
    </td>
    <td></td>
  </tr>

  <!-- Variable Text: -->
  <tr id="field_network_ip_adrs" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Static IP Address:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>wviewPlug IP address (aaa.bbb.ccc.ddd)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_network_ip_adrs id=field_network_ip_adrs size='16' value=
        '<?php echo $field_network_ip_adrs ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Variable Text: -->
  <tr id="field_network_mask" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Netmask:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>wviewPlug net mask (i.e., 255.255.255.0)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_network_mask id=field_network_mask size='16' value=
        '<?php echo $field_network_mask ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Variable Text: -->
  <tr id="field_network_gw" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Gateway IP Address:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>wviewPlug gateway address (aaa.bbb.ccc.ddd)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_network_gw id=field_network_gw size='16' value=
        '<?php echo $field_network_gw ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Variable Text: -->
  <tr id="field_network_dns1" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Primary DNS Server:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>IP address (aaa.bbb.ccc.ddd)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_network_dns1 id=field_network_dns1 size='16' value=
        '<?php echo $field_network_dns1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Variable Text: -->
  <tr id="field_network_dns2" style="display: <?php if ($field_network_dhcp != "yes") echo ""; else echo "none"; ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Secondary DNS Server:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>IP address (aaa.bbb.ccc.ddd)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_network_dns2 id=field_network_dns2 size='16' value=
        '<?php echo $field_network_dns2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Space: -->
  <tr>    
    <td height="20" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td></td>
  </tr>

  <tr id="netupdate" style="display: <?php
                if ($field_network_debian == true)
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF">
        <input id="network" type="submit" 
         value='Save Changes'>
    </td>
    <td></td>
  </tr>
  </form>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="20" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="top" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Admin Password</font></td>
    <td width="6" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="500" valign="top" bgcolor="#FFFFFF"></td>
    <td width="180" valign="top" bgcolor="#FFFFFF"></td>
    <td width="10" valign="top" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="5" valign="top" bgcolor="#6666CC" align="center"> 
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          There is only one admin account. The password is stored as an MD5 hash, so there is no way to get a reminder.
        </font>
        <br>
      </div>
    </td>
  </tr>

  <form method=post enctype=multipart/form-data action=password_update.php>
  <tr>
      <td height="30" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
      <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
      <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>
      <td valign="top" bgcolor="#FFFFFF"> 
        <?php if (isset($_GET['mismatch'])) echo "<font color=\"red\">Passwords did not match!</font><br>"; ?>
        <font color="blue">New administrator password:</font>
      </td>
      <td valign="top" bgcolor="#FFFFFF">
        <input class=mainForm type=password name=field_Admin_Password1 id=field_Admin_Password1 size='20' value=''>
      </td>
      <td></td>
  </tr>

  <tr>
      <td height="30" align="right" valign="top" bgcolor="#E7E7E7">&nbsp;</td>
      <td valign="top" bgcolor="#E7E7E7">&nbsp;</td>
      <td valign="top" bgcolor="#FFFFFF">&nbsp;</td>
      <td valign="top" bgcolor="#FFFFFF"> 
        <font color="blue">Repeat password:</font>
      </td>
      <td valign="top" bgcolor="#FFFFFF">
        <input class=mainForm type=password name=field_Admin_Password2 id=field_Admin_Password2 size='20' value=''>
      </td>
      <td></td>
  </tr>

  <!-- Space: -->
  <tr>    
    <td height="20" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td></td>
  </tr>

  <tr> 
    <td height="30" align="right" valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#E7E7E7"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF"></td>
    <td valign="top" bgcolor="#FFFFFF">
        <input id="network" type="submit" 
         value='Save Changes'>
    </td>
    <td></td>
  </tr>
  </form>

  <!-- Footer: -->
  <tr> 
    <td height="25" colspan="2" valign="middle" align="center" bgcolor="#000000" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">
      <font color="#FFFFFF" style="font-size: 9pt; font-weight: 700" face="Arial">
        <a href="system_status.php?logout=1">Logout</a>
      </font>
    </td>
    <td valign="top" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#000000" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
  </tr>
</table>
</div>
</body>
</html>
