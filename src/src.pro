QT        += core gui xml

TARGET     = LineCounter
DESTDIR    = ../bin

TEMPLATE   = app

SOURCES   += main.cpp\
             wndMain.cpp \
             LCFilter.cpp \
             LCApp.cpp \
             LCTreeItem.cpp \
    LCExport.cpp

HEADERS   += wndMain.h \
             LCFilter.h \
             LCApp.h \
             LCTreeItem.h \
    LCExport.h

FORMS     += wndMain.ui

RESOURCES += ../res.qrc

RC_FILE    = ../res.rc
