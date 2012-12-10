QT				+= core gui widgets

TARGET			 = CodeStat
DESTDIR			 = ../bin
TEMPLATE		 = app
QMAKE_LFLAGS	+= -static-libgcc -static-libstdc++

SOURCES			+= main.cpp\
				   filter.cpp \
				   filterdialog.cpp \
				   export.cpp \
				   mainwindow.cpp

HEADERS			+= filter.h \
				   treeitem.h \
				   filterdialog.h \
				   export.h \
				   mainwindow.h

FORMS			+= mainwindow.ui \
				   filterdialog.ui

RESOURCES		+= ../res.qrc

RC_FILE			 = ../res.rc
