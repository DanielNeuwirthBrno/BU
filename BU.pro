#-------------------------------------------------
#
# Project created by QtCreator 2018-03-17T14:30:22
#
#-------------------------------------------------

QT       += core gui sql xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BU
TEMPLATE = app

SOURCES += bank.cpp \
           bankwindow.cpp \
           database.cpp \
           homebanking.cpp \
           logwindow.cpp \
           main.cpp \
           mainwindow.cpp \
           s5object.cpp

HEADERS += authcombo.h  \
           bank.h \
           bankwindow.h \
           database.h \
           homebanking.h \
           iconlabel.h \
           logwindow.h \
           mainwindow.h \
           s5object.h \
           ui/ui_bankwindow.h \
           ui/ui_logwindow.h \
           ui/ui_mainwindow.h

DISTFILES += notes.txt

RESOURCES += resource.qrc

win32:RC_ICONS += app.ico
win32:VERSION = 1.1.0.0
win32:RC_LANG = 0x0405
win32:QMAKE_TARGET_COPYRIGHT = "Daniel Neuwirth"
