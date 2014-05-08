TEMPLATE = app
LANGUAGE = C++
CONFIG += release warn_on thread qt

HEADERS += mainform.ui.h \
	afedit.h \
	formutils.h \
	archiveFile.h
SOURCES += main.cpp \
	formutils.cpp \
	archiveFile.cpp
FORMS = mainform.ui \
	aboutafedit.ui
IMAGES = ../images/filenew \
	../images/fileopen \
	../images/filesave \
	../images/print \
	../images/undo \
	../images/redo \
	../images/editcut \
	../images/editcopy \
	../images/editpaste \
	../images/searchfind

QMAKE_CFLAGS += -DAFEDIT_VERSION="0.50"
QMAKE_CXXFLAGS += -DAFEDIT_VERSION="0.50"
TARGET = ../bin/afedit





unix{
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}
