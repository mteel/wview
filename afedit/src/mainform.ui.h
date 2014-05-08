/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "afedit.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "formutils.h"
#include ".ui/aboutafedit.h"

static QString  FullPathName;
static char*    AFEDIT_COL_HEADERS[AFEDIT_COLUMNS] =
{
    " Type ",
    "Interval ",
    "IconFlags ",
    "MoreFlags ",
    "PackedTime ",
    " OutTemp  ",
    "HiOutTemp ",
    "LowOutTemp ",
    "insideTemp ",
    "Barometer ",
    " OutHum  ",
    "InsideHum ",
    " Rain  ",
    "HiRainRate ",
    "WindSpeed ",
    "HiWindSpeed ",
    "WindDir ",
    "HiWindDir ",
    "NumSamples ",
    "SolarRad ",
    "HiSolarRad ",
    "  UV  ",
    " hiUV ",
    "LeafTemp1 ",
    "LeafTemp2 ",
    "LeafTemp3 ",
    "LeafTemp4 ",
    "NewSensor1 ",
    "NewSensor2 ",
    "NewSensor3 ",
    "NewSensor4 ",
    "NewSensor5 ",
    "NewSensor6 ",
    "NewSensor7 ",
    "Forecast ",
    "   ET   ",
    "SoilTemp1 ",
    "SoilTemp2 ",
    "SoilTemp3 ",
    "SoilTemp4 ",
    "SoilTemp5 ",
    "SoilTemp6 ",
    "SoilMoist1 ",
    "SoilMoist2 ",
    "SoilMoist3 ",
    "SoilMoist4 ",
    "SoilMoist5 ",
    "SoilMoist6 ",
    "LeafWet1 ",
    "LeafWet2 ",
    "LeafWet3 ",
    "LeafWet4 ",
    "ExtraTemp1 ",
    "ExtraTemp2 ",
    "ExtraTemp3 ",
    "ExtraTemp4 ",
    "ExtraTemp5 ",
    "ExtraTemp6 ",
    "ExtraTemp7 ",
    "ExtraHum1 ",
    "ExtraHum2 ",
    "ExtraHum3 ",
    "ExtraHum4 ",
    "ExtraHum5 ",
    "ExtraHum6 ",
    "ExtraHum7 "
};
    
static char*    AFEDIT_COL_HELPERS[AFEDIT_COLUMNS] =
{
    "decimal: Record Data Type (always = 1)",
    "decimal: minutes in the archive record",
    "hexidecimal: Icon associated with this record and edit flags",
    "hexidecimal: Tx ID, etc.",
    "decimal: minutes past midnight of the end of the archive period",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: inches Hg",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "hexidecimal: number of clicks + rain collector type code",
    "decimal: number of clicks per hour",
    "decimal: miles per hour",
    "decimal: miles per hour",
    "decimal: direction code (0-15, 255)",
    "decimal: direction code (0-15, 255)",
    "decimal: number of valid ISS packets containing wind data",
    "decimal: watts per meter squared",
    "decimal: watts per meter squared",
    "decimal: UV index (0 - 15)",
    "decimal: UV index (0 - 15)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: New Sensor 1",
    "decimal: New Sensor 2",
    "decimal: New Sensor 3",
    "decimal: New Sensor 4",
    "decimal: New Sensor 5",
    "decimal: New Sensor 6",
    "decimal: New Sensor 7",
    "decimal: forecast code during the archive interval",
    "decimal: inches",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: centibars of dryness",
    "decimal: centibars of dryness",
    "decimal: centibars of dryness",
    "decimal: centibars of dryness",
    "decimal: centibars of dryness",
    "decimal: centibars of dryness",
    "decimal: Leaf Wetness code (0-15, 255)",
    "decimal: Leaf Wetness code (0-15, 255)",
    "decimal: Leaf Wetness code (0-15, 255)",
    "decimal: Leaf Wetness code (0-15, 255)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: degrees (-90 - 160)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)",
    "decimal: percent (0 - 100)"
};

static struct ColumnRanges
{
    float       min;
    float       max;
} ValueChecker[AFEDIT_COLUMNS] =
{
    { 1.0, 1.0 },
    { 5.0, 30.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 5.0, 1440.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { 28.0, 33.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 65535.0 },
    { 0.0, 50000.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 255.0 },
    { 0.0, 255.0 },
    { 0.0, 32767.0 },
    { 0.0, 2000.0 },
    { 0.0, 2000.0 },
    { 0.0, 15.0 },
    { 0.0, 15.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 65535.0 },
    { 0.0, 200.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 200.0 },
    { 0.0, 255.0 },
    { 0.0, 255.0 },
    { 0.0, 255.0 },
    { 0.0, 255.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { -90.0, 160.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 },
    { 0.0, 100.0 }
};

void MainForm::init()
{
    FullPathName = QString::null;
    for (int i = 0; i < AFEDIT_COLUMNS; i ++)
    {
        archiveTable->setColumnStretchable (i, false);
        archiveTable->setItem (0, i, new QTableItem (archiveTable, QTableItem::WhenCurrent, tr(AFEDIT_COL_HEADERS[i])));
        archiveTable->adjustColumn (i);
        archiveTable->clearCell (0, i);
        archiveTable->horizontalHeader()->setLabel (i, tr(AFEDIT_COL_HEADERS[i]));
    }
    archiveTable->hideColumn (AFEDIT_COLUMNS);

    QString     HelpText = AFEDIT_COL_HELPERS[archiveTable->currentColumn()];
    lineEdit_Hint->setText (HelpText);

}


void MainForm::fileOpen()
{
    QString         fullPath, Caption;

    if (formUtilsGetFile (fullPath))
    {
        FullPathName = fullPath;
        Caption = "wview Archive File Editor: ";
        Caption += FullPathName;
        setCaption (Caption);

        formUtilsPopulateTable (FullPathName, archiveTable);
    }
    return;
}


void MainForm::fileSave()
{
    if (FullPathName != QString::null)
    {
        // Write out the new file
        formUtilsWriteFile (FullPathName, archiveTable);
    }
}


void MainForm::fileSaveAs()
{
    QString         fullPath, Caption;

    if (formUtilsGetSaveAsFile (fullPath))
    {
        FullPathName = fullPath;
        Caption = "wview Archive File Editor: ";
        Caption += FullPathName;
        setCaption (Caption);

        // Write out the new file
        formUtilsWriteFile (FullPathName, archiveTable);
    }
    return;

}


void MainForm::fileExit()
{
    exit (0);
}


void MainForm::helpContents()
{

}


void MainForm::helpAbout()
{
    AboutAFEdit     AboutDialog;
    QString         TempStr;

    TempStr.sprintf ("Version %.2f", AFEDIT_VERSION);
    AboutDialog.VersionLabel->setText (TempStr);
    AboutDialog.exec();
}

void MainForm::archiveTable_selectionChanged()
{
    QString     HelpText = AFEDIT_COL_HELPERS[archiveTable->currentColumn()];
    lineEdit_Hint->setText (HelpText);
}

void MainForm::archiveTable_valueChanged( int row, int col )
{
    float       ValToCheck;
    QString     field = archiveTable->text(row, col);

    if (field == QString::null)
    {
        return;
    }

    if (col == 2 || col == 3 || col == 12)
    {
        // hex value
        ValToCheck = (float)field.toUShort(0, 16);
    }
    else
    {
        ValToCheck = field.toFloat();
    }
    
    // range check it
    if ((ValToCheck < ValueChecker[col].min) || (ValToCheck > ValueChecker[col].max))
    {
        // squawk about it
        field.sprintf ("New value: %.1f is OUT OF RANGE (%.1f - %.1f) for column %s:\n"
                       "You should fix this value now at row %d, column %d.",
                       ValToCheck,
                       ValueChecker[col].min,
                       ValueChecker[col].max,
                       AFEDIT_COL_HEADERS[col],
                       row + 1,
                       col + 1);
        QMessageBox::information (this, "wview Archive Editor", field);
    }
}
