QT				+= testlib

CONFIG			+= console
CONFIG			-= app_bundle

TEMPLATE		 = app

SOURCES			+=  ../src/filter.cpp \
					filtertests.cpp

QMAKE_CXXFLAGS	+= -std=c++11

HEADERS			+=  ../src/filter.h
