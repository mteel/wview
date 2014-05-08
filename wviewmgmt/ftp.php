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
      window.location = "ftp.php";
  }
  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_ftp.php") ?>

<form method=post enctype=multipart/form-data action=process_ftp.php>

<DIV align=center>
<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" id="AutoNumber11" width="960" height="104">
  <tr>
    <td colspan="3" align="center" valign="center" width="960" height="100" bgcolor="#6666CC"> 
      <img border="0" src="wview-100x100.png">
    </td>
  </tr>

  <tr> 
    <td colspan="3" valaign="center"> <img border="0" src="imgs/blueline.gif" width="960" height="11"></td>
  </tr>
</table>
   
<!-- Header -->
<table height=70 cellSpacing=0 cellPadding=0 width=960 bgColor=black border=0>

  <tr> 
    <!-- Page Name: -->
    <td width="130" height=70 rowspan="3" align=middle bordercolor="#000000" bgColor=black style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">FTP</font></H3></td>

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
		  <td bgcolor="#000000" style="font-style: normal; font-variant: normal; font-weight: normal; font-size: 10pt; color: black"> 
            <p style="margin-bottom: 4"><b><a class="mainmenu" href="alarms.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              Alarms</a></b></td>

          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              FTP</font></b> </td>

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
      <font face="Arial" style="font-size: 10pt" color="#000000">Connection</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valaign="top" bgcolor="#6666CC" align="center"> 
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">FTP Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>FTP server hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_FTP_Hostname id=field_FTP_Hostname size='40' value=
        '<?php echo $field_FTP_Hostname ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">FTP Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>User name on remote FTP server</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_FTP_Username id=field_FTP_Username size='20' value=
        '<?php echo $field_FTP_Username ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">FTP Password:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Password on remote FTP server</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_FTP_Password id=field_FTP_Password size='20' value=
        '<?php echo $field_FTP_Password ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Optional Remote Transfer Directory (relative to the ftp login directory, no leading /)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_FTP_Remote_Dir id=field_FTP_Remote_Dir size='60' value=
        '<?php echo $field_FTP_Remote_Dir ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Checkbox: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>
    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Use EPSV transfer mode (if supported)?</label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=checkbox name=field_FTP_Use_Passive[] id=field_FTP_Use_Passive_option_1 value="yes" 
        <?php if ($field_FTP_Use_Passive == "yes") echo "checked"; ?>>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Global transmit interval (mins):
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_FTP_Interval id=field_FTP_Interval size='10' value=
        '<?php echo $field_FTP_Interval ?>'>
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
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 1</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_1) || count(glob($field_FTP_Source_1)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_1 id=field_FTP_Source_1 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_1 id=field_FTP_Source_1 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 2</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_2) || count(glob($field_FTP_Source_2)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_2 id=field_FTP_Source_2 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_2 id=field_FTP_Source_2 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 3</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_3) || count(glob($field_FTP_Source_3)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_3 id=field_FTP_Source_3 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_3 id=field_FTP_Source_3 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 4</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_4) || count(glob($field_FTP_Source_4)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_4 id=field_FTP_Source_4 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_4 id=field_FTP_Source_4 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 5</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_5) || count(glob($field_FTP_Source_5)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_5 id=field_FTP_Source_5 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_5 id=field_FTP_Source_5 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 6</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_6) || count(glob($field_FTP_Source_6)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_6 id=field_FTP_Source_6 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_6 id=field_FTP_Source_6 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_6 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 7</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_7) || count(glob($field_FTP_Source_7)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_7 id=field_FTP_Source_7 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_7 id=field_FTP_Source_7 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_7 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 8</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_8) || count(glob($field_FTP_Source_8)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_8 id=field_FTP_Source_8 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_8 id=field_FTP_Source_8 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_8 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 9</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="2" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_9) || count(glob($field_FTP_Source_9)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_9 id=field_FTP_Source_9 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_9 id=field_FTP_Source_9 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_9 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 10</font></td>
    <td width="6" valaign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valaign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="3" valaign="top" bgcolor="#6666CC" align="center"> 
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#E7E7E7"></td>
    <td valaign="center" bgcolor="#FFFFFF"></td>

    <td valaign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File(s) (can have wildcards):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
        <span class=infobox>Source file (relative to $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valaign="center" bgcolor="#FFFFFF">
<?php if(empty($field_FTP_Source_10) || count(glob($field_FTP_Source_10)) > 0 ) { ?>
      <input class=mainForm type=text name=field_FTP_Source_10 id=field_FTP_Source_10 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_FTP_Source_10 id=field_FTP_Source_10 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_FTP_Source_10 ?>'>
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
    <td valaign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valaign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
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
    <td valaign="center" bgcolor="#6666CC" bordercolor="#E7E7E7" bordercolorlight="#E7E7E7" bordercolordark="#E7E7E7">&nbsp;</td>
    <td valaign="center" bgcolor="#000000"> 
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
