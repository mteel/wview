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
      window.location = "alarms.php";
  }
  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_alarms.php") ?>

<form method=post enctype=multipart/form-data action=process_alarms.php>

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
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">Alarms</font></H3></td>

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
        ?>
        Weather Server and Site Generator</span></font></b>
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
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-top: 0; margin-bottom: 4"><b><a class="mainmenu" href="station.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Station</a></b> </td>
          <td height="28" bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p align="center" style="margin-bottom: 4"> <b><a class="mainmenu" href="file_generation.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              File<br>Generation</a></b> </td>

          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              Alarms</font></b> </td>

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
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Settings</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="4" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable Metric Units for Alarms?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Metric[] id=field_Alarms_Metric_option_1 value="yes" 
        <?php if ($field_Alarms_Metric == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Test an alarm by faking it when alarms are initialized?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Do_Test[] id=field_Alarms_Do_Test_option_1 value="yes" 
        <?php if ($field_Alarms_Do_Test == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Which alarm (1 - 10) to test:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Alarm index to test</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Do_Test_Number id=field_Alarms_Do_Test_Number size='20' value=
        '<?php echo $field_Alarms_Do_Test_Number ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 1</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_1 id=field_Alarms_Type_1 size='40' value=
        '<?php echo $field_Alarms_Type_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_1[] id=field_Alarms_Max_1_option_1 value="yes" 
        <?php if ($field_Alarms_Max_1 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_1 id=field_Alarms_Threshold_1 size='10' value=
        '<?php echo $field_Alarms_Threshold_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_1 id=field_Alarms_Abatement_1 size='10' value=
        '<?php echo $field_Alarms_Abatement_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_1) || is_executable($field_Alarms_Execute_1 )) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_1 id=field_Alarms_Execute_1 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_1 id=field_Alarms_Execute_1 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 2</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_2 id=field_Alarms_Type_2 size='40' value=
        '<?php echo $field_Alarms_Type_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_2[] id=field_Alarms_Max_2_option_1 value="yes" 
        <?php if ($field_Alarms_Max_2 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_2 id=field_Alarms_Threshold_2 size='10' value=
        '<?php echo $field_Alarms_Threshold_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_2 id=field_Alarms_Abatement_2 size='10' value=
        '<?php echo $field_Alarms_Abatement_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_2) || is_executable($field_Alarms_Execute_2) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_2 id=field_Alarms_Execute_2 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_2 id=field_Alarms_Execute_2 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 3</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_3 id=field_Alarms_Type_3 size='40' value=
        '<?php echo $field_Alarms_Type_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_3[] id=field_Alarms_Max_3_option_1 value="yes" 
        <?php if ($field_Alarms_Max_3 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_3 id=field_Alarms_Threshold_3 size='10' value=
        '<?php echo $field_Alarms_Threshold_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_3 id=field_Alarms_Abatement_3 size='10' value=
        '<?php echo $field_Alarms_Abatement_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_3) || is_executable($field_Alarms_Execute_3) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_3 id=field_Alarms_Execute_3 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_3 id=field_Alarms_Execute_3 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_3 ?>'>
    </td>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 4</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_4 id=field_Alarms_Type_4 size='40' value=
        '<?php echo $field_Alarms_Type_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_4[] id=field_Alarms_Max_4_option_1 value="yes" 
        <?php if ($field_Alarms_Max_4 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_4 id=field_Alarms_Threshold_4 size='10' value=
        '<?php echo $field_Alarms_Threshold_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_4 id=field_Alarms_Abatement_4 size='10' value=
        '<?php echo $field_Alarms_Abatement_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_4) || is_executable($field_Alarms_Execute_4) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_4 id=field_Alarms_Execute_4 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_4 id=field_Alarms_Execute_4 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 5</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_5 id=field_Alarms_Type_5 size='40' value=
        '<?php echo $field_Alarms_Type_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_5[] id=field_Alarms_Max_5_option_1 value="yes" 
        <?php if ($field_Alarms_Max_5 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_5 id=field_Alarms_Threshold_5 size='10' value=
        '<?php echo $field_Alarms_Threshold_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_5 id=field_Alarms_Abatement_5 size='10' value=
        '<?php echo $field_Alarms_Abatement_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_5) || is_executable($field_Alarms_Execute_5) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_5 id=field_Alarms_Execute_5 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_5 id=field_Alarms_Execute_5 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 6</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_6 id=field_Alarms_Type_6 size='40' value=
        '<?php echo $field_Alarms_Type_6 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_6[] id=field_Alarms_Max_6_option_1 value="yes" 
        <?php if ($field_Alarms_Max_6 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_6 id=field_Alarms_Threshold_6 size='10' value=
        '<?php echo $field_Alarms_Threshold_6 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_6 id=field_Alarms_Abatement_6 size='10' value=
        '<?php echo $field_Alarms_Abatement_6 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_6) || is_executable($field_Alarms_Execute_6) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_6 id=field_Alarms_Execute_6 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_6 id=field_Alarms_Execute_6 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_6 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 7</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_7 id=field_Alarms_Type_7 size='40' value=
        '<?php echo $field_Alarms_Type_7 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_7[] id=field_Alarms_Max_7_option_1 value="yes" 
        <?php if ($field_Alarms_Max_7 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_7 id=field_Alarms_Threshold_7 size='10' value=
        '<?php echo $field_Alarms_Threshold_7 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_7 id=field_Alarms_Abatement_7 size='10' value=
        '<?php echo $field_Alarms_Abatement_7 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_7) || is_executable($field_Alarms_Execute_7) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_7 id=field_Alarms_Execute_7 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_7 id=field_Alarms_Execute_7 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_7 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 8</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_8 id=field_Alarms_Type_8 size='40' value=
        '<?php echo $field_Alarms_Type_8 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_8[] id=field_Alarms_Max_8_option_1 value="yes" 
        <?php if ($field_Alarms_Max_8 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_8 id=field_Alarms_Threshold_8 size='10' value=
        '<?php echo $field_Alarms_Threshold_8 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_8 id=field_Alarms_Abatement_8 size='10' value=
        '<?php echo $field_Alarms_Abatement_8 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_8) || is_executable($field_Alarms_Execute_8) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_8 id=field_Alarms_Execute_8 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_8 id=field_Alarms_Execute_8 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_8 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 9</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_9 id=field_Alarms_Type_9 size='40' value=
        '<?php echo $field_Alarms_Type_9 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_9[] id=field_Alarms_Max_9_option_1 value="yes" 
        <?php if ($field_Alarms_Max_9 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_9 id=field_Alarms_Threshold_9 size='10' value=
        '<?php echo $field_Alarms_Threshold_9 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_9 id=field_Alarms_Abatement_9 size='10' value=
        '<?php echo $field_Alarms_Abatement_9 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_9) || is_executable($field_Alarms_Execute_9) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_9 id=field_Alarms_Execute_9 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_9 id=field_Alarms_Execute_9 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_9 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Alarm 10</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="280" valign="center" bgcolor="#FFFFFF"></td>
    <td width="400" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="7" valign="center" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center">
        <font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
          &nbsp;
        </font>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Type:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
                    <span class=infobox>
            0 - Barometer             <br>
		    1 - InsideTemp            <br>
		    2 - InsideHumidity        <br>
		    3 - OutsideTemp           <br>
		    4 - WindSpeed             <br>
		    5 - TenMinuteAvgWindSpeed <br>
		    6 - WindDirection         <br>
		    7 - OutsideHumidity       <br>
		    8 - RainRate              <br>
		    9 - StormRain             <br>
		   10 - DayRain               <br>
		   11 - MonthRain             <br>
		   12 - YearRain              <br>
		   13 - TxBatteryStatus       <br>
		   14 - ConsoleBatteryVoltage <br>
		   15 - DewPoint              <br>
		   16 - WindChill             <br>
		   17 - HeatIndex             <br>
		   18 - Radiation             <br>
		   19 - UV                    <br>
		   20 - ET                    <br>
		   21 - ExtraTemp1            <br>
		   22 - ExtraTemp2            <br>
		   23 - ExtraTemp3            <br>
		   24 - SoilTemp1             <br>
		   25 - SoilTemp2             <br>
		   26 - SoilTemp3             <br>
		   27 - SoilTemp4             <br>
		   28 - LeafTemp1             <br>
		   29 - LeafTemp2             <br>
		   30 - ExtraHumid1           <br>
		   31 - ExtraHumid2           <br>
          </span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Type_10 id=field_Alarms_Type_10 size='40' value=
        '<?php echo $field_Alarms_Type_10 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Is This an Upper Bound Alarm?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Alarms_Max_10[] id=field_Alarms_Max_10_option_1 value="yes" 
        <?php if ($field_Alarms_Max_10 == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Threshold:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Upper/lower bound value (threshold) (float or integer)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Threshold_10 id=field_Alarms_Threshold_10 size='10' value=
        '<?php echo $field_Alarms_Threshold_10 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Abatement (seconds):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Number of seconds to suppress alarms after an alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Alarms_Abatement_10 id=field_Alarms_Abatement_10 size='10' value=
        '<?php echo $field_Alarms_Abatement_10 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarm Execution Target:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Full path to the shell script or binary to execute when the alarm triggers</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_Alarms_Execute_10) || is_executable($field_Alarms_Execute_10) ) { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_10 id=field_Alarms_Execute_10 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_Alarms_Execute_10 id=field_Alarms_Execute_10 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_Alarms_Execute_10 ?>'>
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
        <a href="system_status.php?logout=1">Logout</a>
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
    <td valign="center" bgcolor="#000000"> 
      <div align="center"> 
        <center>
        </center>
      </div>
    </td>
  </tr>
</table>
</div>
</form>
</body>
</html>
