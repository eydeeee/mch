#-------------------------------------------------
#
# Project created by QtCreator 2014-01-22T20:59:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = mchecker
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    optionsdialog.cpp \
    accountmanager.cpp \
    accounteditordialog.cpp \
    settingsmanager.cpp \
    notificationwindow.cpp \
    stringdecoder.cpp

HEADERS  += mainwindow.h \
    optionsdialog.h \
    accountmanager.h \
    accounteditordialog.h \
    settingsmanager.h \
    notificationwindow.h \
    structs.h \
    stringdecoder.h

FORMS    += mainwindow.ui \
    optionsdialog.ui \
    accounteditordialog.ui \
    notificationwindow.ui

unix:INCLUDEPATH += /usr/include/glib-2.0
unix:INCLUDEPATH += /usr/include/glib-2.0/glib
unix:INCLUDEPATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include/

unix:LIBS += -L"/usr/lib/x86_64-linux-gnu" -lcrypto
unix:LIBS += -L"/usr/lib/x86_64-linux-gnu" -lssl

win32:INCLUDEPATH += "$$_PRO_FILE_PWD_/ssl64/include"
win32:INCLUDEPATH += "$$_PRO_FILE_PWD_/iconv/include"

win32:LIBS += -L"$$_PRO_FILE_PWD_/ssl64/lib" -llibeay32
win32:LIBS += -L"$$_PRO_FILE_PWD_/ssl64/lib" -lssleay32

win32:LIBS += -L"$$_PRO_FILE_PWD_/iconv/lib64" -llibiconv

#win32:LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib" -lWs2_32
#win32:LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib" -lMswsock
#win32:LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib" -lAdvApi32

win32:LIBS += -lWs2_32
win32:LIBS += -lMswsock
win32:LIBS += -lAdvApi32
win32:LIBS += -luser32

win32:RC_FILE = rctmp.rc
