QT        += core gui widgets xml

TARGET     = LineCounter
DESTDIR    = ../bin

TEMPLATE   = app

SOURCES   += main.cpp\
			 filter.cpp \
			 filterdialog.cpp \
			 export.cpp \
			 mainwindow.cpp

HEADERS   += filter.h \
			 treeitem.h \
			 filterdialog.h \
			 export.h \
			 mainwindow.h

FORMS     += mainwindow.ui \
			 filterdialog.ui

RESOURCES += ../res.qrc

RC_FILE    = ../res.rc
