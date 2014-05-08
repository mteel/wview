/***************************************************************************
 *   Copyright (C) 2007 by Mark Teel   *
 *   mteel@teelworks.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "formutils.h"


static QString& ExtractArchiveColumn (ArchiveRecord *record, int column)
{
    static QString  ArcCol;

    ArcCol.truncate (0);

    if (column >= AFEDIT_COLUMNS)
    {
        return ArcCol;
    }

    switch (column)
    {
        case 0: ArcCol.setNum ((USHORT)record->dataType); break;
        case 1: ArcCol.setNum ((USHORT)record->archiveInterval); break;
        case 2: ArcCol.setNum ((USHORT)record->iconFlags, 16); break;
        case 3: ArcCol.setNum ((USHORT)record->moreFlags, 16); break;
        case 4: ArcCol.setNum ((USHORT)record->packedTime); break;
        case 5: ArcCol.setNum ((float)record->outsideTemp/10.0, 'f', 1); break;
        case 6: ArcCol.setNum ((float)record->hiOutsideTemp/10.0, 'f', 1); break;
        case 7: ArcCol.setNum ((float)record->lowOutsideTemp/10.0, 'f', 1); break;
        case 8: ArcCol.setNum ((float)record->insideTemp/10.0, 'f', 1); break;
        case 9: ArcCol.setNum ((float)record->barometer/1000.0, 'f', 3); break;
        case 10: ArcCol.setNum ((float)record->outsideHum/10.0, 'f', 1); break;
        case 11: ArcCol.setNum ((float)record->insideHum/10.0, 'f', 1); break;
        case 12: ArcCol.setNum ((USHORT)record->rain, 16); break;
        case 13: ArcCol.setNum ((USHORT)record->hiRainRate); break;
        case 14: ArcCol.setNum ((float)record->windSpeed/10.0, 'f', 1); break;
        case 15: ArcCol.setNum ((float)record->hiWindSpeed/10.0, 'f', 1); break;
        case 16: ArcCol.setNum ((USHORT)record->windDirection); break;
        case 17: ArcCol.setNum ((USHORT)record->hiWindDirection); break;
        case 18: ArcCol.setNum ((USHORT)record->numWindSamples); break;
        case 19: ArcCol.setNum ((USHORT)record->solarRad); break;
        case 20: ArcCol.setNum ((USHORT)record->hisolarRad); break;
        case 21: ArcCol.setNum ((float)record->UV/10.0, 'f', 1); break;
        case 22: ArcCol.setNum ((float)record->hiUV/10.0, 'f', 1); break;
        case 23: ArcCol.setNum ((USHORT)record->leafTemp[0] - 90); break;
        case 24: ArcCol.setNum ((USHORT)record->leafTemp[1] - 90); break;
        case 25: ArcCol.setNum ((USHORT)record->leafTemp[2] - 90); break;
        case 26: ArcCol.setNum ((USHORT)record->leafTemp[3] - 90); break;
        case 27: ArcCol.setNum ((USHORT)record->newSensors[0]); break;
        case 28: ArcCol.setNum ((USHORT)record->newSensors[1]); break;
        case 29: ArcCol.setNum ((USHORT)record->newSensors[2]); break;
        case 30: ArcCol.setNum ((USHORT)record->newSensors[3]); break;
        case 31: ArcCol.setNum ((USHORT)record->newSensors[4]); break;
        case 32: ArcCol.setNum ((USHORT)record->newSensors[5]); break;
        case 33: ArcCol.setNum ((USHORT)record->newSensors[6]); break;
        case 34: ArcCol.setNum ((USHORT)record->forecast); break;
        case 35: ArcCol.setNum ((float)record->ET/1000.0, 'f', 3); break;
        case 36: ArcCol.setNum ((USHORT)record->soilTemp[0] - 90); break;
        case 37: ArcCol.setNum ((USHORT)record->soilTemp[1] - 90); break;
        case 38: ArcCol.setNum ((USHORT)record->soilTemp[2] - 90); break;
        case 39: ArcCol.setNum ((USHORT)record->soilTemp[3] - 90); break;
        case 40: ArcCol.setNum ((USHORT)record->soilTemp[4] - 90); break;
        case 41: ArcCol.setNum ((USHORT)record->soilTemp[5] - 90); break;
        case 42: ArcCol.setNum ((USHORT)record->soilMoisture[0]); break;
        case 43: ArcCol.setNum ((USHORT)record->soilMoisture[1]); break;
        case 44: ArcCol.setNum ((USHORT)record->soilMoisture[2]); break;
        case 45: ArcCol.setNum ((USHORT)record->soilMoisture[3]); break;
        case 46: ArcCol.setNum ((USHORT)record->soilMoisture[4]); break;
        case 47: ArcCol.setNum ((USHORT)record->soilMoisture[5]); break;
        case 48: ArcCol.setNum ((USHORT)record->leafWetness[0]); break;
        case 49: ArcCol.setNum ((USHORT)record->leafWetness[1]); break;
        case 50: ArcCol.setNum ((USHORT)record->leafWetness[2]); break;
        case 51: ArcCol.setNum ((USHORT)record->leafWetness[3]); break;
        case 52: ArcCol.setNum ((USHORT)record->extraTemp[0] - 90); break;
        case 53: ArcCol.setNum ((USHORT)record->extraTemp[1] - 90); break;
        case 54: ArcCol.setNum ((USHORT)record->extraTemp[2] - 90); break;
        case 55: ArcCol.setNum ((USHORT)record->extraTemp[3] - 90); break;
        case 56: ArcCol.setNum ((USHORT)record->extraTemp[4] - 90); break;
        case 57: ArcCol.setNum ((USHORT)record->extraTemp[5] - 90); break;
        case 58: ArcCol.setNum ((USHORT)record->extraTemp[6] - 90); break;
        case 59: ArcCol.setNum ((USHORT)record->extraHum[0]); break;
        case 60: ArcCol.setNum ((USHORT)record->extraHum[1]); break;
        case 61: ArcCol.setNum ((USHORT)record->extraHum[2]); break;
        case 62: ArcCol.setNum ((USHORT)record->extraHum[3]); break;
        case 63: ArcCol.setNum ((USHORT)record->extraHum[4]); break;
        case 64: ArcCol.setNum ((USHORT)record->extraHum[5]); break;
        case 65: ArcCol.setNum ((USHORT)record->extraHum[6]); break;
    }

    return ArcCol;
}

static void insertArchiveColumn (ArchiveRecord *record, int column, QString& field)
{
    if (column > AFEDIT_COLUMNS)
    {
        return;
    }

    switch (column)
    {
        case 0: record->dataType = field.toUShort(); break;
        case 1: record->archiveInterval = field.toUShort(); break;
        case 2: record->iconFlags = field.toUShort(0, 16); break;
        case 3: record->moreFlags = field.toUShort(0, 16); break;
        case 4: record->packedTime = field.toUShort(); break;
        case 5: record->outsideTemp = (short)(field.toFloat() * 10); break;
        case 6: record->hiOutsideTemp = (short)(field.toFloat() * 10); break;
        case 7: record->lowOutsideTemp = (short)(field.toFloat() * 10); break;
        case 8: record->insideTemp = (short)(field.toFloat() * 10); break;
        case 9: record->barometer = (USHORT)(field.toFloat() * 1000); break;
        case 10: record->outsideHum = (USHORT)(field.toFloat() * 10); break;
        case 11: record->insideHum = (USHORT)(field.toFloat() * 10); break;
        case 12: record->rain = field.toUShort(0, 16); break;
        case 13: record->hiRainRate = field.toUShort(); break;
        case 14: record->windSpeed = (USHORT)(field.toFloat() * 10); break;
        case 15: record->hiWindSpeed = (USHORT)(field.toFloat() * 10); break;
        case 16: record->windDirection = field.toUShort(); break;
        case 17: record->hiWindDirection = field.toUShort(); break;
        case 18: record->numWindSamples = field.toUShort(); break;
        case 19: record->solarRad = field.toUShort(); break;
        case 20: record->hisolarRad = field.toUShort(); break;
        case 21: record->UV = (USHORT)(field.toFloat() * 10); break;
        case 22: record->hiUV = (USHORT)(field.toFloat() * 10); break;
        case 23: record->leafTemp[0] = field.toShort() + 90; break;
        case 24: record->leafTemp[1] = field.toShort() + 90; break;
        case 25: record->leafTemp[2] = field.toShort() + 90; break;
        case 26: record->leafTemp[3] = field.toShort() + 90; break;
        case 27: record->newSensors[0] = field.toShort(); break;
        case 28: record->newSensors[1] = field.toShort(); break;
        case 29: record->newSensors[2] = field.toShort(); break;
        case 30: record->newSensors[3] = field.toShort(); break;
        case 31: record->newSensors[4] = field.toShort(); break;
        case 32: record->newSensors[5] = field.toShort(); break;
        case 33: record->newSensors[6] = field.toShort(); break;
        case 34: record->forecast = field.toUShort(); break;
        case 35: record->ET = (USHORT)(field.toFloat() * 1000); break;
        case 36: record->soilTemp[0] = field.toShort() + 90; break;
        case 37: record->soilTemp[1] = field.toShort() + 90; break;
        case 38: record->soilTemp[2] = field.toShort() + 90; break;
        case 39: record->soilTemp[3] = field.toShort() + 90; break;
        case 40: record->soilTemp[4] = field.toShort() + 90; break;
        case 41: record->soilTemp[5] = field.toShort() + 90; break;
        case 42: record->soilMoisture[0] = field.toUShort(); break;
        case 43: record->soilMoisture[1] = field.toUShort(); break;
        case 44: record->soilMoisture[2] = field.toUShort(); break;
        case 45: record->soilMoisture[3] = field.toUShort(); break;
        case 46: record->soilMoisture[4] = field.toUShort(); break;
        case 47: record->soilMoisture[5] = field.toUShort(); break;
        case 48: record->leafWetness[0] = field.toUShort(); break;
        case 49: record->leafWetness[1] = field.toUShort(); break;
        case 50: record->leafWetness[2] = field.toUShort(); break;
        case 51: record->leafWetness[3] = field.toUShort(); break;
        case 52: record->extraTemp[0] = field.toShort() + 90; break;
        case 53: record->extraTemp[1] = field.toShort() + 90; break;
        case 54: record->extraTemp[2] = field.toShort() + 90; break;
        case 55: record->extraTemp[3] = field.toShort() + 90; break;
        case 56: record->extraTemp[4] = field.toShort() + 90; break;
        case 57: record->extraTemp[5] = field.toShort() + 90; break;
        case 58: record->extraTemp[6] = field.toShort() + 90; break;
        case 59: record->extraHum[0] = field.toUShort(); break;
        case 60: record->extraHum[1] = field.toUShort(); break;
        case 61: record->extraHum[2] = field.toUShort(); break;
        case 62: record->extraHum[3] = field.toUShort(); break;
        case 63: record->extraHum[4] = field.toUShort(); break;
        case 64: record->extraHum[5] = field.toUShort(); break;
        case 65: record->extraHum[6] = field.toUShort(); break;
    }
}


bool formUtilsGetFile (QString& fullPath)
{
    QString     str;

    fullPath.truncate (0);

    QFileDialog* fd = new QFileDialog (0, "open file dialog", TRUE);
    fd->setMode (QFileDialog::ExistingFile);
    fd->setFilter ("Archive Files (*.wlk *.WLK)");
    fd->setViewMode (QFileDialog::Detail);
    fd->setDir ("/home");
    fd->setCaption ("Choose Archive File to Edit");
    if (fd->exec() == QDialog::Accepted)
    {
        fullPath = fd->selectedFile ();
        return true;
    }
    else
    {
        return false;
    }
}

bool formUtilsGetSaveAsFile (QString& fullPath)
{
    fullPath.truncate (0);

    QFileDialog* fd = new QFileDialog (0, "save as file dialog", TRUE);
    fd->setMode (QFileDialog::AnyFile);
    fd->setFilter ("Archive Files (*.wlk *.WLK)");
    fd->setViewMode (QFileDialog::Detail);
    fd->setDir ("/home");
    fd->setCaption ("Save As");
    if (fd->exec() == QDialog::Accepted)
    {
        fullPath = fd->selectedFile ();
        if ((fullPath.right(4) != ".wlk") && (fullPath.right(4) != ".WLK"))
        {
            fullPath += ".wlk";
        }
        return true;
    }
    else
    {
        return false;
    }
}

static void setRowHeader (QTable* table, int row, ArchiveRecord* record, int day)
{
    QString     Header;

    Header.sprintf ("%2.2d %2.2d:%2.2d",
                    day,
                    record->packedTime/60,
                    record->packedTime%60);
    table->verticalHeader()->setLabel (row, Header);
}

void formUtilsPopulateTable (QString& fullPath, QTable* table)
{
    bool            Done = false;
    ArchiveRecord*  recordStore;
    int             row = 0;
    int             column, day;
    CArchiveFile    ArchiveFile ((char *)fullPath.ascii ());

    
    while (! Done)
    {
        recordStore = ArchiveFile.GetNext (&day);
        if (recordStore != 0)
        {
            // Set the header
            setRowHeader (table, row, recordStore, day);

            // Set the hidden day field
            QString TempStr;
            TempStr.sprintf ("%d", day);
            table->setItem (row, AFEDIT_COLUMNS, new QTableItem (table, QTableItem::WhenCurrent, TempStr));

            // Populate a row in our table
            for (column = 0; column < AFEDIT_COLUMNS; column ++)
            {
                table->setItem (row,
                                column,
                                new QTableItem (table,
                                                QTableItem::WhenCurrent,
                                                ExtractArchiveColumn (recordStore, column)));
            }

            row ++;
        }
        else
        {
            Done = true;
        }
    }
}

void formUtilsWriteFile (QString& fullPath, QTable* table)
{
    ArchiveRecord   recordStore;
    int             day;
    CArchiveFile    ArchiveFile ((char *)fullPath.ascii (), true);

    if (! ArchiveFile.IsValid())
    {
        return;
    }

    for (int row = 0; table->text(row, AFEDIT_COLUMNS) != QString::null; row ++)
    {
        // Build a record from this row's data
        memset (&recordStore, 0xFF, sizeof(recordStore));
        for (int column = 0; column < AFEDIT_COLUMNS; column ++)
        {
            QString field = table->text(row, column);
            if (field != QString::null)
            {
                insertArchiveColumn (&recordStore, column, field);
            }
        }

        // Write it to the file
        day = (table->text(row, AFEDIT_COLUMNS)).toUShort();
        ArchiveFile.PutNext (&recordStore, day);
    }
}
