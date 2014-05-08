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
      window.location = "services.php";
  }
  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_services.php") ?>

<form method=post enctype=multipart/form-data action=process_services.php>

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
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">Services</font></H3></td>

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

          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              Services</font></b> </td>

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
      <font face="Arial" style="font-size: 10pt" color="#000000">wview Processes</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        You can choose which wview processes are enabled based on the services you require. <br><br>
      </div>
    </td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable file generation (HTML/XML/graphics/etc.) (htmlgend)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_htmlgend[] id=field_Enable_htmlgend_option_1 value="yes" 
        <?php if ($field_Enable_htmlgend == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable Alarms (wvalarmd)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_wvalarmd[] id=field_Enable_wvalarmd_option_1 value="yes" 
        <?php if ($field_Enable_wvalarmd == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable CWOP Submission (wvcwopd)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_wvcwopd[] id=field_Enable_wvcwopd_option_1 value="yes" 
        <?php if ($field_Enable_wvcwopd == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable HTTP (Wunderground/Weatherforyou) Submission (wvhttpd)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_wvhttpd[] id=field_Enable_wvhttpd_option_1 value="yes" 
        <?php if ($field_Enable_wvhttpd == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Radio Buttons: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Export to remote host?&nbsp;</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=radio name=field_Export_Remote_Type id=field_Export_Remote_Type_option_1 
        value="FTP" <?php if ($field_Export_Remote_Type == "FTP") echo "checked"; ?>>
        <label class=formFieldOption for="field_Export_Remote_Type_option_1">FTP</label>&nbsp;
      <input class=mainForm type=radio name=field_Export_Remote_Type id=field_Export_Remote_Type_option_2 
        value="SSH" <?php if ($field_Export_Remote_Type == "SSH") echo "checked"; ?>>
        <label class=formFieldOption for="field_Export_Remote_Type_option_2">SSH</label>&nbsp;
      <input class=mainForm type=radio name=field_Export_Remote_Type id=field_Export_Remote_Type_option_3 
        value="None" <?php if ($field_Export_Remote_Type == "None") echo "checked"; ?>>
        <label class=formFieldOption for="field_Export_Remote_Type_option_3">None</label>&nbsp;
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable process monitoring (wvpmond)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_wvpmond[] id=field_Enable_wvpmond_option_1 value="yes" 
        <?php if ($field_Enable_wvpmond == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="12" align="right" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="center" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Logging</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="9" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        You can choose which wview processes log verbosely. Error logs will always be enabled. <br><br>
      </div>
    </td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Station Interface (wviewd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wviewd_Verbose[] id=field_wviewd_Verbose_option_1 value="yes" 
        <?php if ($field_wviewd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">File generation (htmlgend) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_htmlgend_Verbose[] id=field_htmlgend_Verbose_option_1 value="yes" 
        <?php if ($field_htmlgend_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Alarms (wvalarmd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wvalarmd_Verbose[] id=field_wvalarmd_Verbose_option_1 value="yes" 
        <?php if ($field_wvalarmd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">FTP (wviewftpd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wviewftpd_Verbose[] id=field_wviewftpd_Verbose_option_1 value="yes" 
        <?php if ($field_wviewftpd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH (wviewsshd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wviewsshd_Verbose[] id=field_wviewsshd_Verbose_option_1 value="yes" 
        <?php if ($field_wviewsshd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">CWOP (wvcwopd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wvcwopd_Verbose[] id=field_wvcwopd_Verbose_option_1 value="yes" 
        <?php if ($field_wvcwopd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">HTTP (wvhttpd) verbose logging?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_wvhttpd_Verbose[] id=field_wvhttpd_Verbose_option_1 value="yes" 
        <?php if ($field_wvhttpd_Verbose == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="12" align="right" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="center" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Email Alerts</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="6" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        Receive email if significant problems are detected by wview. <br><br>
      </div>
    </td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable email notification for wview alerts?&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>sendmail and sendEmail must be installed on the wview server</span></a></label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Enable_Email[] id=field_Enable_Email_option_1 value="yes" 
        <?php if ($field_Enable_Email == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">From Email address:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Where to send alert emails</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Email_From_Address id=field_Email_From_Address size='20' value=
        '<?php echo $field_Email_From_Address ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">To Email address:&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Where to send alert emails</span></a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_Email_Address id=field_Email_Address size='20' value=
        '<?php echo $field_Email_Address ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Enable test email?&nbsp;<a class=info href=#><img src=imgs/tip_small.png border=0><span class=infobox>Send test email when wview starts</span></a></label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_Send_Test_Email[] id=field_Send_Test_Email_option_1 value="yes" 
        <?php if ($field_Send_Test_Email == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Horizontal rule: -->
  <tr> 
    <td height="12" align="right" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td valign="center" bgcolor="#FFFFFF">&nbsp;</td>

    <td colspan="2" valign="center" bgcolor="#FFFFFF"> 
      <center>
        <hr align="center" size="1" color="#b5b5e6" noshade>
      </center>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="20" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Process Monitoring</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="500" valign="center" bgcolor="#FFFFFF"></td>
    <td width="180" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="7" valign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        Seconds to wait before restarting a non-responsive process (0 disables). <br><br>
      </div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">wviewd:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_ProcMon_wviewd id=field_ProcMon_wviewd size='10' value=
        '<?php echo $field_ProcMon_wviewd ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">htmlgend:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_ProcMon_htmlgend id=field_ProcMon_htmlgend size='10' value=
        '<?php echo $field_ProcMon_htmlgend ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">wvalarmd:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_ProcMon_wvalarmd id=field_ProcMon_wvalarmd size='10' value=
        '<?php echo $field_ProcMon_wvalarmd ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">wvcwopd:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_ProcMon_wvcwopd id=field_ProcMon_wvcwopd size='10' value=
        '<?php echo $field_ProcMon_wvcwopd ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">wvhttpd:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_ProcMon_wvhttpd id=field_ProcMon_wvhttpd size='10' value=
        '<?php echo $field_ProcMon_wvhttpd ?>'>
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
        <a href="services.php?logout=1">Logout</a>
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
