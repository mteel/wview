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
      window.location = "ssh.php";
  }
  // -->
  </script>

</head>


<body link="#B5B5E6" vlink="#B5B5E6" alink="#B5B5E6">

<?php require("preload_ssh.php") ?>

<form method=post enctype=multipart/form-data action=process_ssh.php>

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
      <H3 style="margin-top: 1; margin-bottom: 1" align="center"> <font color="#FFFFFF" face="Arial">SSH</font></H3></td>

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
          <td bgcolor="#000000" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"><b> <a class="mainmenu" href="ftp.php" style="font-size: 8pt; text-decoration: none; font-weight:700"> 
              FTP</a></b> </td>

          <td bgcolor="#6666CC" style="border-style: none; border-width: medium; font-style:normal; font-variant:normal; font-weight:normal; font-size:10pt; color:black"> 
            <p style="margin-bottom: 4"> <b><font color="#FFFFFF" style="font-size: 8pt"> 
              SSH</font></b> </td>

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
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 1</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valign="top" bgcolor="#6666CC" align="center">
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File or Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Local Source directory (normally $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_SSH_Source_1) || file_exists($field_SSH_Source_1) ) { ?>
      <input class=mainForm type=text name=field_SSH_Source_1 id=field_SSH_Source_1 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_SSH_Source_1 id=field_SSH_Source_1 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_SSH_Source_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Update Interval:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>How often to synchronize the remote server (minutes)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Interval_1 id=field_SSH_Interval_1 size='10' value=
        '<?php echo $field_SSH_Interval_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Host_1 id=field_SSH_Host_1 size='40' value=
        '<?php echo $field_SSH_Host_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH Port:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Port_1 id=field_SSH_Port_1 size='10' value=
        '<?php echo $field_SSH_Port_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote SSH username</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_UserName_1 id=field_SSH_UserName_1 size='32' value=
        '<?php echo $field_SSH_UserName_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Destination Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote destination path, relative to the ssh login directory (no leading /)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Destination_1 id=field_SSH_Destination_1 size='60' value=
        '<?php echo $field_SSH_Destination_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Timeout (secs):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>SSH session timeout value</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Timeout_1 id=field_SSH_Timeout_1 size='12' value=
        '<?php echo $field_SSH_Timeout_1 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 2</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valign="top" bgcolor="#6666CC" align="center">
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File or Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Local Source directory (normally $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_SSH_Source_2) || file_exists($field_SSH_Source_2) ) { ?>
      <input class=mainForm type=text name=field_SSH_Source_2 id=field_SSH_Source_2 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_SSH_Source_2 id=field_SSH_Source_2 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_SSH_Source_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Update Interval:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>How often to synchronize the remote server (minutes)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Interval_2 id=field_SSH_Interval_2 size='10' value=
        '<?php echo $field_SSH_Interval_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Host_2 id=field_SSH_Host_2 size='40' value=
        '<?php echo $field_SSH_Host_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH Port:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Port_2 id=field_SSH_Port_2 size='10' value=
        '<?php echo $field_SSH_Port_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote SSH username</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_UserName_2 id=field_SSH_UserName_2 size='32' value=
        '<?php echo $field_SSH_UserName_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Destination Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote destination path, relative to the ssh login directory (no leading /)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Destination_2 id=field_SSH_Destination_2 size='60' value=
        '<?php echo $field_SSH_Destination_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Timeout (secs):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>SSH session timeout value</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Timeout_2 id=field_SSH_Timeout_2 size='12' value=
        '<?php echo $field_SSH_Timeout_2 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 3</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valign="top" bgcolor="#6666CC" align="center">
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File or Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Local Source directory (normally $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_SSH_Source_3) || file_exists($field_SSH_Source_3) ) { ?>
      <input class=mainForm type=text name=field_SSH_Source_3 id=field_SSH_Source_3 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_SSH_Source_3 id=field_SSH_Source_3 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_SSH_Source_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Update Interval:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>How often to synchronize the remote server (minutes)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Interval_3 id=field_SSH_Interval_3 size='10' value=
        '<?php echo $field_SSH_Interval_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Host_3 id=field_SSH_Host_3 size='40' value=
        '<?php echo $field_SSH_Host_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH Port:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Port_3 id=field_SSH_Port_3 size='10' value=
        '<?php echo $field_SSH_Port_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote SSH username</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_UserName_3 id=field_SSH_UserName_3 size='32' value=
        '<?php echo $field_SSH_UserName_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Destination Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote destination path, relative to the ssh login directory (no leading /)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Destination_3 id=field_SSH_Destination_3 size='60' value=
        '<?php echo $field_SSH_Destination_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Timeout (secs):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>SSH session timeout value</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Timeout_3 id=field_SSH_Timeout_3 size='12' value=
        '<?php echo $field_SSH_Timeout_3 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 4</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="8" valign="top" bgcolor="#6666CC" align="center">
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File or Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Local Source directory (normally $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_SSH_Source_4) || file_exists($field_SSH_Source_4) ) { ?>
      <input class=mainForm type=text name=field_SSH_Source_4 id=field_SSH_Source_4 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_SSH_Source_4 id=field_SSH_Source_4 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_SSH_Source_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Update Interval:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>How often to synchronize the remote server (minutes)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Interval_4 id=field_SSH_Interval_4 size='10' value=
        '<?php echo $field_SSH_Interval_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Host_4 id=field_SSH_Host_4 size='40' value=
        '<?php echo $field_SSH_Host_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH Port:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Port_4 id=field_SSH_Port_4 size='10' value=
        '<?php echo $field_SSH_Port_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote SSH username</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_UserName_4 id=field_SSH_UserName_4 size='32' value=
        '<?php echo $field_SSH_UserName_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Destination Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote destination path, relative to the ssh login directory (no leading /)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Destination_4 id=field_SSH_Destination_4 size='60' value=
        '<?php echo $field_SSH_Destination_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Timeout (secs):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>SSH session timeout value</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Timeout_4 id=field_SSH_Timeout_4 size='12' value=
        '<?php echo $field_SSH_Timeout_4 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Section Heading: -->
  <tr> 
    <td width="124" height="12" valign="center" align="right" bgcolor="#E7E7E7">
      <font face="Arial" style="font-size: 10pt" color="#000000">Rule 5</font></td>
    <td width="6" valign="center" bgcolor="#E7E7E7">&nbsp;</td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="200" valign="center" bgcolor="#FFFFFF"></td>
    <td width="480" valign="center" bgcolor="#FFFFFF"></td>
    <td width="10" valign="center" bgcolor="#FFFFFF"></td>
    <td width="130" rowspan="9" valign="top" bgcolor="#6666CC" align="center">
      <br>
      <br>
      <div align="center"><font face="Arial" style="font-size: 8pt" color="#FFFFFF"> 
        </font></div>
    </td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Source File or Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Local Source directory (normally $prefix/var/wview/img)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
<?php if(empty($field_SSH_Source_5) || file_exists($field_SSH_Source_5) ) { ?>
      <input class=mainForm type=text name=field_SSH_Source_5 id=field_SSH_Source_5 size='60' value=
<?php } else { ?>
      <input class=mainForm type=text name=field_SSH_Source_5 id=field_SSH_Source_5 size='60' style="background-color:red" value=
<?php }; ?>
        '<?php echo $field_SSH_Source_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Update Interval:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>How often to synchronize the remote server (minutes)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Interval_5 id=field_SSH_Interval_5 size='10' value=
        '<?php echo $field_SSH_Interval_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Remote Host:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>hostname or IP address</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Host_5 id=field_SSH_Host_5 size='40' value=
        '<?php echo $field_SSH_Host_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">SSH Port:
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Port_5 id=field_SSH_Port_5 size='10' value=
        '<?php echo $field_SSH_Port_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Username:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote SSH username</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_UserName_5 id=field_SSH_UserName_5 size='32' value=
        '<?php echo $field_SSH_UserName_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr> 
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF"> 
      <label class="formFieldQuestion">Destination Directory:&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>Remote destination path, relative to the ssh login directory (no leading /)</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Destination_5 id=field_SSH_Destination_5 size='60' value=
        '<?php echo $field_SSH_Destination_5 ?>'>
    </td>
    <td></td>
  </tr>

  <!-- Text: -->
  <tr>
    <td height="30" align="right" valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#E7E7E7"></td>
    <td valign="center" bgcolor="#FFFFFF"></td>

    <td valign="center" bgcolor="#FFFFFF">
      <label class="formFieldQuestion">Timeout (secs):&nbsp;
        <a class=info href=#><img src=imgs/tip_small.png border=0>
          <span class=infobox>SSH session timeout value</span>
        </a>
      </label>
    </td>
    <td valign="center" bgcolor="#FFFFFF">
      <input class=mainForm type=text name=field_SSH_Timeout_5 id=field_SSH_Timeout_5 size='12' value=
        '<?php echo $field_SSH_Timeout_5 ?>'>
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
