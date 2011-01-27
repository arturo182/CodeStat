QT       += core gui xml

TARGET    = LineCounter
DESTDIR   = ../bin

TEMPLATE  = app

SOURCES  += main.cpp\
            wndMain.cpp \
            LCFilter.cpp \
            LCApp.cpp

HEADERS  += wndMain.h \
            LCFilter.h \
            LCApp.h

FORMS    += wndMain.ui
