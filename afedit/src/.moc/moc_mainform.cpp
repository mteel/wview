/****************************************************************************
** MainForm meta object code from reading C++ file 'mainform.h'
**
** Created: Mon Jul 9 22:50:57 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../.ui/mainform.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MainForm::className() const
{
    return "MainForm";
}

QMetaObject *MainForm::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MainForm( "MainForm", &MainForm::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MainForm::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainForm", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MainForm::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainForm", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MainForm::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"fileOpen", 0, 0 };
    static const QUMethod slot_1 = {"fileSave", 0, 0 };
    static const QUMethod slot_2 = {"fileSaveAs", 0, 0 };
    static const QUMethod slot_3 = {"fileExit", 0, 0 };
    static const QUMethod slot_4 = {"helpContents", 0, 0 };
    static const QUMethod slot_5 = {"helpAbout", 0, 0 };
    static const QUMethod slot_6 = {"archiveTable_selectionChanged", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"archiveTable_valueChanged", 2, param_slot_7 };
    static const QUMethod slot_8 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "fileOpen()", &slot_0, QMetaData::Public },
	{ "fileSave()", &slot_1, QMetaData::Public },
	{ "fileSaveAs()", &slot_2, QMetaData::Public },
	{ "fileExit()", &slot_3, QMetaData::Public },
	{ "helpContents()", &slot_4, QMetaData::Public },
	{ "helpAbout()", &slot_5, QMetaData::Public },
	{ "archiveTable_selectionChanged()", &slot_6, QMetaData::Public },
	{ "archiveTable_valueChanged(int,int)", &slot_7, QMetaData::Public },
	{ "languageChange()", &slot_8, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"MainForm", parentObject,
	slot_tbl, 9,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MainForm.setMetaObject( metaObj );
    return metaObj;
}

void* MainForm::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MainForm" ) )
	return this;
    return QMainWindow::qt_cast( clname );
}

bool MainForm::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: fileOpen(); break;
    case 1: fileSave(); break;
    case 2: fileSaveAs(); break;
    case 3: fileExit(); break;
    case 4: helpContents(); break;
    case 5: helpAbout(); break;
    case 6: archiveTable_selectionChanged(); break;
    case 7: archiveTable_valueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 8: languageChange(); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MainForm::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MainForm::qt_property( int id, int f, QVariant* v)
{
    return QMainWindow::qt_property( id, f, v);
}

bool MainForm::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
