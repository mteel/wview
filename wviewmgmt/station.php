<?php require("password_protect.php") ?>
<html>

<head>
  <meta name="Pragma" content="No-Cache">
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
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
      window.location = "station.php";
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

  <!-- Show/Hide config items based on station selection  -->
  function hideShowStationObjs(station, isSerial, isEthernet)
  {
     objWmr = document.getElementById('wmrchan').style;
     objWmr = document.getElementById('rxcheck').style;
     objwlinkip = document.getElementById('wlinkip').style;
     objwlinkarchive = document.getElementById('wlinkarchive').style;
     objdevice = document.getElementById('devicename').style;
     objhost = document.getElementById('hostname').style;
     objport = document.getElementById('portnumber').style;
     if (station == 'Oregon Scientific WMR9XX')
     {
        show('wmrchan');
        show('interface');
        hide('rxcheck');
        hide('wlinkip');
        hide('serialdtr');
        hide('misc_header_small');
        show('misc_header_big');
        if (isSerial)
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
        else if (isEthernet)
        {
           hide('devicename');
           show('hostname');
           show('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           show('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
        else
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
     }
     else if ((station == 'Oregon Scientific WMRUSB') || 
              (station == 'Fine Offset WH1080') ||
              (station == 'Honeywell TE923') ||
              (station == 'Simulator'))
     {
        hide('wmrchan');
        hide('rxcheck');
        hide('wlinkip');
        hide('serialdtr');
        hide('interface');
        show('misc_header_small');
        hide('misc_header_big');
        hide('devicename');
        hide('hostname');
        hide('portnumber');
        hide('comm_header_giant');
        hide('comm_header_big');
        hide('comm_header_medium');
        hide('comm_header_small');
        show('comm_header_tiny');
     }
     else if (station == 'Davis Vantage Pro')
     {
        show('wlinkip');
        show('wlinkarchive');
        show('rxcheck');
        hide('wmrchan');
        hide('serialdtr');
        show('interface');
        show('misc_header_big');
        hide('misc_header_small');
        hide('location_header_normal');
        hide('elevation');
        hide('latitude');
        hide('longitude');
        if (isSerial)
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           show('comm_header_big');
           hide('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
        else if (isEthernet)
        {
           hide('devicename');
           show('hostname');
           show('portnumber');
           show('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
        else
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           show('comm_header_big');
           hide('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
     }
     else if ((station == 'Viasala WXT510') || (station == 'Texas Weather Instruments'))
     {
        hide('rxcheck');
        hide('wlinkip');
        hide('wmrchan');
        show('serialdtr');
        show('interface');
        show('misc_header_big');
        hide('misc_header_small');
        if (isSerial)
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
        else if (isEthernet)
        {
           hide('devicename');
           show('hostname');
           show('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           show('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
        else
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
     }
     else
     {
        hide('rxcheck');
        hide('wmrchan');
        hide('wlinkip');
        hide('serialdtr');
        show('interface');
        hide('misc_header_big');
        show('misc_header_small');
        if (isSerial)
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
        else if (isEthernet)
        {
           hide('devicename');
           show('hostname');
           show('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           show('comm_header_medium');
           hide('comm_header_small');
           hide('comm_header_tiny');
        }
        else
        {
           show('devicename');
           hide('hostname');
           hide('portnumber');
           hide('comm_header_giant');
           hide('comm_header_big');
           hide('comm_header_medium');
           show('comm_header_small');
           hide('comm_header_tiny');
        }
     }
  }

  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_station.php") ?>

<form method=post enctype=multipart/form-data action=process_station.php>

<DIV align=center>
<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber11" width="960" height="104">
  <tr>
    <td colspan="3" align="center" valign="center" width="960" height="100" bgcolor="#6666CC"> 
      <img border="0" src="wview-100x100.png">
    </td>
  </tr>

  <tr> 
    <td colspan="3" valign="center"> <img border="0" src="imgs/blueline.gif" width="960" height="11"></td>
  </tr>
</table>
   
<!-- Header -->
<table height=70 cellSpacing=0 cellPadding=0 width=960 bgColor=black border=0>

  <tr> 
    <!-- Page Name: -->
    <td width="130" height=70 rowspan="3" align=middle bordercolor="#000000" bgColor=black style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">Station</font></H3></td>

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
          else if ($field_Station_Type == "Oregon Scientific WMRUSB")
              echo "Oregon Scientific WMRUSB";
          else if ($field_Station_Type == "Fine Offset WH1080")
              echo "Fine Offset WH1080";
          else if ($field_Station_Type == "Honeywell TE923")
              echo "Honeywell TE923";
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
        ?> Weather Server and Site Generator</span></font></b>
    </td>

    <!-- wview Version: -->
    <td vAlign=center width=130 bgColor=#000000 style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black" bordercolor="#000000"> 
      <p align="center"><font color="#FFFFFF"> <span style="font-size: 8pt"><b>Version <?php echo $wview_version ?></b></span></font> 
    </td>
  </tr>

  <tr> 
    <td height=36 colspan="2" vAlign=center bordercolor="#000000" bgColor=#000000 style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black">
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
          <td height="28" bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p align="center" style="margin-bottom: 4"> <b><a class="mainmenu" href="system_status.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              System<br>Status</a></b></td>
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-top: 0; margin-bottom: 4"><b><a class="mainmenu" href="services.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Services</a></b> </td>

          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              Station</font></b> </td>

          <td height="28" bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p align="center" style="margin-bottom: 4"> <b><a class="mainmenu" href="file_generation.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              File<br>
              Generation</a></b> </td>
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
  <tr id="comm_header_tiny" style=<?php 
                if (!strcmp($field_Station_Type, "Oregon Scientific WMRUSB") ||
                    !strcmp($field_Station_Type, "Fine Offset WH1080") ||
                    !strcmp($field_Station_Type, "Honeywell TE923") ||
                    !strcmp($field_Station_Type, "Simulator"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Communication</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        The simulator has no station interface. The WMRUSB/WH1080/TE923 weather stations 
        only support a native USB interface. No interface configuration is required.<br></font>
      </div>
    </td>
  </tr>
  <tr id="comm_header_small" style=<?php 
                if (strcmp($field_Station_Type, "Davis Vantage Pro") &&
                    strcmp($field_Station_Type, "Simulator") &&
                    strcmp($field_Station_Type, "Oregon Scientific WMRUSB") &&
                    strcmp($field_Station_Type, "Fine Offset WH1080") &&
                    strcmp($field_Station_Type, "Honeywell TE923") &&
                    strcmp($field_Station_Interface, "ethernet"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Communication</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        The weather station can be connected using a serial or ethernet interface.<br></font>
      </div>
    </td>
  </tr>
  <tr id="comm_header_medium" style=<?php
                if (strcmp($field_Station_Type, "Davis Vantage Pro") &&
                     strcmp($field_Station_Type, "Simulator") &&
                     strcmp($field_Station_Type, "Oregon Scientific WMRUSB") &&
                     strcmp($field_Station_Type, "Fine Offset WH1080") &&
                     strcmp($field_Station_Type, "Honeywell TE923") &&
                     !strcmp($field_Station_Interface, "ethernet"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Communication</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="5" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        The weather station can be connected using a serial or ethernet interface.<br></font>
      </div>
    </td>
  </tr>
  <tr id="comm_header_big" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro") && 
                    !strcmp($field_Station_Interface, "serial"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Communication</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        The weather station can be connected using a serial or ethernet interface.<br></font>
      </div>
    </td>
  </tr>
  <tr id="comm_header_giant" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro") && 
                    !strcmp($field_Station_Interface, "ethernet"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Communication</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="7" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        The weather station can be connected using a serial or ethernet interface.<br></font>
      </div>
    </td>
  </tr>


  <!-- Drop-down list: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Type:</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <select class=mainForm name=field_Station_Type id=field_Station_Type 
        onchange="hideShowStationObjs(field_Station_Type.value, 
                                      field_Station_Interface_option_1.checked, 
                                      field_Station_Interface_option_2.checked)">
        <option <?php if ($field_Station_Type == "Davis Vantage Pro") echo "selected"; ?> 
          value="Davis Vantage Pro">Davis Vantage Pro</option>
        <option <?php if ($field_Station_Type == "La Crosse WS-23XX") echo "selected"; ?> 
          value="La Crosse WS-23XX">La Crosse WS-23XX</option>
        <option <?php if ($field_Station_Type == "Oregon Scientific WMR9XX") echo "selected"; ?> 
          value="Oregon Scientific WMR9XX">Oregon Scientific WMR9XX</option>
        <option <?php if ($field_Station_Type == "Oregon Scientific WMRUSB") echo "selected"; ?> 
          value="Oregon Scientific WMRUSB">Oregon Scientific WMRUSB</option>
        <option <?php if ($field_Station_Type == "Viasala WXT510") echo "selected"; ?> 
          value="Viasala WXT510">Viasala WXT510</option>
        <option <?php if ($field_Station_Type == "Texas Weather Instruments") echo "selected"; ?> 
          value="Texas Weather Instruments">Texas Weather Instruments</option>
        <option <?php if ($field_Station_Type == "Fine Offset WH1080") echo "selected"; ?> 
          value="Fine Offset WH1080">Fine Offset WH1080</option>
        <option <?php if ($field_Station_Type == "Honeywell TE923") echo "selected"; ?> 
          value="Honeywell TE923">Honeywell TE923</option>
        <option <?php if ($field_Station_Type == "Virtual") echo "selected"; ?> 
          value="Virtual">Virtual</option>
        <option <?php if ($field_Station_Type == "Simulator") echo "selected"; ?> 
          value="Simulator">Simulator</option>
      </select>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr id="wlinkip" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
    <label class="formFieldQuestion">Enable Weatherlink IP Interface:
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Only select this if you have a Vantage Pro station with the WeatherlinkIP datalogger.
            The default port number for the WeatherlinkIP datalogger is 22222.</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Station_WLIP[] id=field_Station_WLIP_option_1 value="yes" 
        <?php if ($field_Station_WLIP == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr id="wlinkarchive" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
    <label class="formFieldQuestion">Retrieve archive records from console (if not enabled, they will be autogenerated):
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>
            Only deselect this if you have a Vantage Pro station and don't want to use the console-generated archive records. 
            This will also prevent retrieval of missed archive records in the datalogger if the wview server loses connection
            with the VP console.
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Station_Retrieve_Archive[] id=field_Station_Retrieve_Archive_option_1 value="yes" 
        <?php if ($field_Station_Retrieve_Archive == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Radio Buttons: -->
  <tr id="interface" style=<?php
                if (!strcmp($field_Station_Type, "Oregon Scientific WMRUSB") ||
                    !strcmp($field_Station_Type, "Fine Offset WH1080") ||
                    !strcmp($field_Station_Type, "Honeywell TE923") ||
                    !strcmp($field_Station_Type, "Simulator"))
                    echo $NONE; 
                else 
                    echo $EMPTY;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Interface:&nbsp;</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=radio name=field_Station_Interface id=field_Station_Interface_option_1 
        value="serial" <?php if ($field_Station_Interface == "serial") echo "checked"; ?>
        onclick="hideShowStationObjs(field_Station_Type.value, 
                                      field_Station_Interface_option_1.checked, 
                                      field_Station_Interface_option_2.checked)">
        <label class=formFieldOption for="field_Station_Interface_option_1">Serial</label>&nbsp;&nbsp;&nbsp;
      <input class=mainForm type=radio name=field_Station_Interface id=field_Station_Interface_option_2 
        value="ethernet" <?php if ($field_Station_Interface == "ethernet") echo "checked"; ?>
        onclick="hideShowStationObjs(field_Station_Type.value, 
                                      field_Station_Interface_option_1.checked, 
                                      field_Station_Interface_option_2.checked)">
        <label class=formFieldOption for="field_Station_Interface_option_2">Ethernet</label>&nbsp;
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr id="devicename" style=<?php
                if (strcmp($field_Station_Type, "Oregon Scientific WMRUSB") &&
                    strcmp($field_Station_Type, "Fine Offset WH1080") &&
                    strcmp($field_Station_Type, "Honeywell TE923") &&
                    strcmp($field_Station_Type, "Simulator") &&
                    !strcmp($field_Station_Interface, "serial"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Device:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>/dev/ttyUSB0, /dev/ttyS0, etc.</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Station_Device) || file_exists($field_Station_Device)) { ?>
      <input class=mainForm type=text name=field_Station_Device id=field_Station_Device size='20' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Station_Device id=field_Station_Device size='20' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Station_Device ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr id="hostname" style=<?php
                if (strcmp($field_Station_Type, "Oregon Scientific WMRUSB") &&
                    strcmp($field_Station_Type, "Fine Offset WH1080") &&
                    strcmp($field_Station_Type, "Honeywell TE923") &&
                    strcmp($field_Station_Type, "Simulator") &&
                    !strcmp($field_Station_Interface, "ethernet"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Host (ethernet only):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>IP address or hostname</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Host id=field_Station_Host size='20' value=
        '<?php echo $field_Station_Host ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr id="portnumber" style=<?php
                if (strcmp($field_Station_Type, "Oregon Scientific WMRUSB") &&
                    strcmp($field_Station_Type, "Fine Offset WH1080") &&
                    strcmp($field_Station_Type, "Honeywell TE923") &&
                    strcmp($field_Station_Type, "Simulator") &&
                    !strcmp($field_Station_Interface, "ethernet"))
                    echo $EMPTY; 
                else 
                    echo $NONE;
                ?>> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Port (ethernet only):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Port number</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Port id=field_Station_Port size='20' value=
        '<?php echo $field_Station_Port ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Storms</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        Configure storm criteria. <br></font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Rain Season Start (1-12):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Month your rain season begins</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Rain_Season_Start id=field_Station_Rain_Season_Start size='10' value=
        '<?php echo $field_Station_Rain_Season_Start ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Rain Rate Storm Trigger Start:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Decimal rain rate which will start a storm event</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Storm_Trigger_Start id=field_Station_Storm_Trigger_Start size='10' value=
        '<?php echo $field_Station_Storm_Trigger_Start ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Idle Hours Storm Trigger Stop:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Number of hours with no rain that ends the rain storm event</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Storm_Trigger_Stop id=field_Station_Storm_Trigger_Stop size='10' value=
        '<?php echo $field_Station_Storm_Trigger_Stop ?>'>
    </td>
    <td></td>
  </tr>


  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Presets</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        Preset accumulated values if data was missed<br></font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Rain YTD Preset Value:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Additional yearly rainfall not captured by wview</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Rain_YTD id=field_Station_Rain_YTD size='10' value=
        '<?php echo $field_Station_Rain_YTD ?>'>
    </td>
    <td></td>
  </tr>


  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station ET YTD Preset Value:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Additional yearly ET not captured by wview</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_ET_YTD id=field_Station_ET_YTD size='10' value=
        '<?php echo $field_Station_ET_YTD ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">YTD Preset Year:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Year to apply preset values</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_YTD_Year id=field_Station_YTD_Year size='10' value=
        '<?php echo $field_Station_YTD_Year ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr id="location_header_normal" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $NONE; 
                else 
                    echo $EMPTY;
                ?>>
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Location</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        &nbsp;<br></font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr id="elevation" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $NONE; 
                else 
                    echo $EMPTY;
                ?>>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Elevation (feet):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Enter the elevation of the station in feet</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Elevation id=field_Station_Elevation size='10' value=
        '<?php echo $field_Station_Elevation ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr id="latitude" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $NONE; 
                else 
                    echo $EMPTY;
                ?>>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Latitude (decimal degrees, N is positive - S is negative):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Weather station latitude (decimal degrees, NORTH is positive - SOUTH is negative)</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Latitude id=field_Station_Latitude size='10' value=
        '<?php echo $field_Station_Latitude ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr id="longitude" style=<?php
                if (!strcmp($field_Station_Type, "Davis Vantage Pro"))
                    echo $NONE; 
                else 
                    echo $EMPTY;
                ?>>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Longitude (decimal degrees, E is positive - W is negative):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Weather station longitude (decimal degrees, EAST is positive - WEST is negative)</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Station_Longitude id=field_Station_Longitude size='10' value=
        '<?php echo $field_Station_Longitude ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Timing</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        &nbsp;<br></font>
      </div>
    </td>
  </tr>

  <!-- Drop-down list: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Archive Interval (minutes):</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <select class=mainForm name=field_Station_Archive_Interval id=field_Station_Archive_Interval>
        <option <?php if ($field_Station_Archive_Interval == "5") echo "selected"; ?> 
          value="5">5</option>
        <option <?php if ($field_Station_Archive_Interval == "10") echo "selected"; ?> 
          value="10">10</option>
        <option <?php if ($field_Station_Archive_Interval == "15") echo "selected"; ?> 
          value="15">15</option>
        <option <?php if ($field_Station_Archive_Interval == "30") echo "selected"; ?> 
          value="30">30</option>
      </select>
    </td>
    <td></td>
  </tr>

  <!-- Drop-down list: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Sensor Polling Interval (seconds):</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <select class=mainForm name=field_Station_Polling_Interval id=field_Station_Polling_Interval>
        <option <?php if ($field_Station_Polling_Interval == "30") echo "selected"; ?> 
          value="30">30</option>
        <option <?php if ($field_Station_Polling_Interval == "60") echo "selected"; ?> 
          value="60">60</option>
        <option <?php if ($field_Station_Polling_Interval == "15") echo "selected"; ?> 
          value="15">15</option>
      </select>
    </td>
    <td></td>
  </tr>

  <!-- Drop-down list: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Data Push Interval (seconds):&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Current conditions data push interval (seconds) - for wvalarmd</span></a></label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <select class=mainForm name=field_Station_Push_Interval id=field_Station_Push_Interval>
        <option <?php if ($field_Station_Push_Interval == "15") echo "selected"; ?> 
          value="15">15</option>
        <option <?php if ($field_Station_Push_Interval == "30") echo "selected"; ?> 
          value="30">30</option>
        <option <?php if ($field_Station_Push_Interval == "60") echo "selected"; ?> 
          value="60">60</option>
        <option <?php if ($field_Station_Push_Interval == "120") echo "selected"; ?> 
          value="120">120</option>
        <option <?php if ($field_Station_Push_Interval == "180") echo "selected"; ?> 
          value="180">180</option>
        <option <?php if ($field_Station_Push_Interval == "240") echo "selected"; ?> 
          value="240">240</option>
        <option <?php if ($field_Station_Push_Interval == "300") echo "selected"; ?> 
          value="300">300</option>
        <option <?php if ($field_Station_Push_Interval == "600") echo "selected"; ?> 
          value="600">600</option>
      </select>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr id="misc_header_small" style="display: <?php
                if (($field_Station_Type != "Oregon Scientific WMR9XX") && 
                    ($field_Station_Type != "Davis Vantage Pro") &&
                    ($field_Station_Type != "Viasala WXT510") &&
                    ($field_Station_Type != "Texas Weather Instruments"))
                    echo ""; 
                else 
                    echo "none";
                ?>">
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Miscellaneous</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        &nbsp;<br></font>
      </div>
    </td>
  </tr>
  <tr id="misc_header_big" style="display: <?php
                if (($field_Station_Type == "Oregon Scientific WMR9XX") ||
                    ($field_Station_Type == "Davis Vantage Pro") ||
                    ($field_Station_Type == "Viasala WXT510") ||
                    ($field_Station_Type == "Texas Weather Instruments"))
                    echo ""; 
                else 
                    echo "none";
                ?>">
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Miscellaneous</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="3" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        &nbsp;<br></font>
      </div>
    </td>
  </tr>

  <!-- Checkbox: -->
  <tr id="rxcheck" style="display: <?php
                if ($field_Station_Type == "Davis Vantage Pro")
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
    <label class="formFieldQuestion">Do RX Check (Vantage Pro/Vue only):
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Only select this if you have a Vantage Pro station and want wireless reception statistics generated</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Station_Do_RX_Check[] id=field_Station_Do_RX_Check_option_1 value="yes" 
        <?php if ($field_Station_Do_RX_Check == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr id="serialdtr" style="display: <?php
                if (($field_Station_Type == "Viasala WXT510") ||
                    ($field_Station_Type == "Texas Weather Instruments"))
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
    <label class="formFieldQuestion">Enable serial DTR toggle during initialization?:
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Only applies to TWI and Vaisala stations. 
            Some serial (or USB-serial) implementations require the DTR line to be toggled in order for the station 
            interface to be started properly. If you have one of these stations and are having interface issues, try 
            changing this setting.</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Station_Do_DTR[] id=field_Station_Do_DTR_option_1 value="yes" 
        <?php if ($field_Station_DTR == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Drop-down list: -->
  <tr id="wmrchan" style="display: <?php
                if ($field_Station_Type == "Oregon Scientific WMR9XX")
                    echo ""; 
                else 
                    echo "none";
                ?>"> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
    <label class="formFieldQuestion">Outside Temperature Channel (WMR9XX only):
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Only select this if you have a WMR9XX station and want temperature to come from one of the extra sensors (pool sensor is not supported) - blank indicates main sensor</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <select class=mainForm name=field_Station_Outside_Channel id=field_Station_Outside_Tempurature_Channel>
        <option <?php
                    if ($field_Station_Outside_Channel == "0")
                        echo "selected";
                ?> 
          value="0"> </option>
        <option <?php
                    if ($field_Station_Outside_Channel == "1")
                        echo "selected";
                ?> 
          value="1">1</option>
        <option <?php
                    if ($field_Station_Outside_Channel == "2")
                        echo "selected";
                ?> 
          value="2">2</option>
        <option <?php
                    if ($field_Station_Outside_Channel == "3")
                        echo "selected";
                ?> 
          value="3">3</option>
      </select>
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

  <!-- Footer: -->
  <tr> 
    <td height="25" colspan="2" valign="middle" align="center" bgcolor="#000000" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">
      <font color="#FFFFFF" style="font-size: 9pt; font-weight: 700" face="Arial">
        <a href="station.php?logout=1">Logout</a>
      </font>
    </td>
    <td valign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="middle" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7"> 
      <table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber15" align="right" height="19">
        <tr> 
          <td width="101" bgcolor="#42498C" align="center"> 
            <input id="saveForm" class="mainForm" type="submit" value="Save Changes" >
          </td>
          <td width="8" align="center" bgcolor="#6666CC">&nbsp;</td>
          <td width="103" bgcolor="#434A8F" align="center">
            <input id="saveForm" class="mainForm" type="button" onclick="reloadPage()" value="Cancel Changes" >
          </td>
        </tr>
      </table>
    </td>
    <td valign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valign="top" bgcolor="#000000" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
  </tr>
</table>
</div>
</form>
</body>
</html>
