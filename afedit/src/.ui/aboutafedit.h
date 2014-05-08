/****************************************************************************
** Form interface generated from reading ui file 'aboutafedit.ui'
**
** Created: Mon Jul 9 22:50:53 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ABOUTAFEDIT_H
#define ABOUTAFEDIT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;

class AboutAFEdit : public QDialog
{
    Q_OBJECT

public:
    AboutAFEdit( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AboutAFEdit();

    QLabel* textLabel1;
    QPushButton* buttonOk;
    QLabel* VersionLabel_2;
    QLabel* VersionLabel;

protected:
    QHBoxLayout* Layout1;

protected slots:
    virtual void languageChange();

};

#endif // ABOUTAFEDIT_H
