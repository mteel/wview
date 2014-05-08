/****************************************************************************
** Form implementation generated from reading ui file 'mainform.ui'
**
** Created: Mon Jul 9 22:50:55 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mainform.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qtable.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

#include "../mainform.ui.h"
/*
 *  Constructs a MainForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
MainForm::MainForm( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "MainForm" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 21, 114 ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );

    textLabel1 = new QLabel( centralWidget(), "textLabel1" );
    textLabel1->setGeometry( QRect( 10, 850, 91, 21 ) );
    textLabel1->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    archiveTable = new QTable( centralWidget(), "archiveTable" );
    archiveTable->setGeometry( QRect( 0, 0, 1320, 840 ) );
    archiveTable->setMouseTracking( FALSE );
    archiveTable->setLineWidth( 2 );
    archiveTable->setNumRows( 8928 );
    archiveTable->setNumCols( 67 );
    archiveTable->setSelectionMode( QTable::Single );

    lineEdit_Hint = new QLineEdit( centralWidget(), "lineEdit_Hint" );
    lineEdit_Hint->setGeometry( QRect( 120, 850, 1120, 21 ) );
    lineEdit_Hint->setAcceptDrops( FALSE );
    lineEdit_Hint->setReadOnly( TRUE );

    // actions
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "" ) ) );
    fileSaveAction = new QAction( this, "fileSaveAction" );
    fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "" ) ) );
    fileSaveAsAction = new QAction( this, "fileSaveAsAction" );
    fileExitAction = new QAction( this, "fileExitAction" );
    helpContentsAction = new QAction( this, "helpContentsAction" );
    helpAboutAction = new QAction( this, "helpAboutAction" );


    // toolbars


    // menubar
    MenuBar = new QMenuBar( this, "MenuBar" );


    fileMenu = new QPopupMenu( this );
    fileOpenAction->addTo( fileMenu );
    fileSaveAction->addTo( fileMenu );
    fileSaveAsAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    MenuBar->insertItem( QString(""), fileMenu, 1 );

    helpMenu = new QPopupMenu( this );
    helpContentsAction->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    MenuBar->insertItem( QString(""), helpMenu, 2 );

    languageChange();
    resize( QSize(1320, 946).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    connect( fileSaveAsAction, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    connect( archiveTable, SIGNAL( selectionChanged() ), this, SLOT( archiveTable_selectionChanged() ) );
    connect( archiveTable, SIGNAL( valueChanged(int,int) ), this, SLOT( archiveTable_valueChanged(int,int) ) );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MainForm::languageChange()
{
    setCaption( tr( "wview Archive File Editor" ) );
    textLabel1->setText( tr( "Hint:" ) );
    fileOpenAction->setText( tr( "Open" ) );
    fileOpenAction->setMenuText( tr( "&Open..." ) );
    fileOpenAction->setAccel( tr( "Ctrl+O" ) );
    fileSaveAction->setText( tr( "Save" ) );
    fileSaveAction->setMenuText( tr( "&Save" ) );
    fileSaveAction->setAccel( tr( "Ctrl+S" ) );
    fileSaveAsAction->setText( tr( "Save As" ) );
    fileSaveAsAction->setMenuText( tr( "Save &As..." ) );
    fileSaveAsAction->setAccel( QString::null );
    fileExitAction->setText( tr( "Exit" ) );
    fileExitAction->setMenuText( tr( "E&xit" ) );
    fileExitAction->setAccel( QString::null );
    helpContentsAction->setText( tr( "Contents" ) );
    helpContentsAction->setMenuText( tr( "&Contents..." ) );
    helpContentsAction->setAccel( QString::null );
    helpAboutAction->setText( tr( "About" ) );
    helpAboutAction->setMenuText( tr( "&About" ) );
    helpAboutAction->setAccel( QString::null );
    if (MenuBar->findItem(1))
        MenuBar->findItem(1)->setText( tr( "&File" ) );
    if (MenuBar->findItem(2))
        MenuBar->findItem(2)->setText( tr( "&Help" ) );
}

