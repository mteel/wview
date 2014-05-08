/****************************************************************************
** Form implementation generated from reading ui file 'aboutafedit.ui'
**
** Created: Mon Jul 9 22:50:56 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "aboutafedit.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a AboutAFEdit as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AboutAFEdit::AboutAFEdit( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AboutAFEdit" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setBackgroundOrigin( QDialog::WindowOrigin );
    setSizeGripEnabled( TRUE );
    setModal( TRUE );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setGeometry( QRect( 80, 60, 350, 41 ) );
    QFont textLabel1_font(  textLabel1->font() );
    textLabel1_font.setPointSize( 20 );
    textLabel1->setFont( textLabel1_font ); 

    QWidget* privateLayoutWidget = new QWidget( this, "Layout1" );
    privateLayoutWidget->setGeometry( QRect( 420, 190, 80, 33 ) );
    Layout1 = new QHBoxLayout( privateLayoutWidget, 0, 6, "Layout1"); 

    buttonOk = new QPushButton( privateLayoutWidget, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    VersionLabel_2 = new QLabel( this, "VersionLabel_2" );
    VersionLabel_2->setGeometry( QRect( 140, 150, 221, 31 ) );
    QFont VersionLabel_2_font(  VersionLabel_2->font() );
    VersionLabel_2_font.setPointSize( 12 );
    VersionLabel_2->setFont( VersionLabel_2_font ); 

    VersionLabel = new QLabel( this, "VersionLabel" );
    VersionLabel->setGeometry( QRect( 140, 110, 221, 31 ) );
    QFont VersionLabel_font(  VersionLabel->font() );
    VersionLabel_font.setPointSize( 12 );
    VersionLabel->setFont( VersionLabel_font ); 
    VersionLabel->setFrameShadow( QLabel::Raised );
    languageChange();
    resize( QSize(516, 238).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AboutAFEdit::~AboutAFEdit()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AboutAFEdit::languageChange()
{
    setCaption( tr( "About wview Archive File Editor" ) );
    textLabel1->setText( tr( "wview Archive File Editor" ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    VersionLabel_2->setText( tr( "(c) 2007, Mark Teel" ) );
    VersionLabel->setText( QString::null );
}

