/****************************************************************************
** Form interface generated from reading ui file 'mainform.ui'
**
** Created: Mon Jul 9 22:50:53 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MAINFORM_H
#define MAINFORM_H

#include <qvariant.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QLabel;
class QTable;
class QLineEdit;

class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~MainForm();

    QLabel* textLabel1;
    QTable* archiveTable;
    QLineEdit* lineEdit_Hint;
    QMenuBar *MenuBar;
    QPopupMenu *fileMenu;
    QPopupMenu *helpMenu;
    QAction* fileOpenAction;
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* fileExitAction;
    QAction* helpContentsAction;
    QAction* helpAboutAction;

public slots:
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void fileExit();
    virtual void helpContents();
    virtual void helpAbout();
    virtual void archiveTable_selectionChanged();
    virtual void archiveTable_valueChanged( int row, int col );

protected:

protected slots:
    virtual void languageChange();

private:
    virtual void init();

};

#endif // MAINFORM_H
