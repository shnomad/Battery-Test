#-------------------------------------------------
#
# Project created by QtCreator 2019-08-30T15:14:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Battery_LifeTime_Test
TEMPLATE = app
#QTPLUGIN +=qtslibplugin
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        control.cpp \
        main.cpp \
        mainwindow.cpp \
        measurement.cpp \
        relay_seed_ddl.cpp \
        settings.cpp \
        loggingcategories.cpp
#        serialcom/serialporttester.cpp \
#        serialcom/serialcomm.cpp \
#        serialcom/serialprotocol3.cpp \
#        serialcom/serialprotocolabstract.cpp \
#        serialcom/stmhidport.cpp \
#        serialcom/stmhidtester.cpp

HEADERS += \
        control.h \
        mainwindow.h \
        measurement.h \
        measurement_param.h \
        relay_seed_ddl.h \
        commondefinition.h \
        settings.h \
        setting_flagname_definition.h \
        builddatetime.h \
        loggingcategories.h
#        serialcom/glucosedownloadprogress.h \
#        serialcom/serialporttester.h \
#        serialcom/serialcomm.h \
#        serialcom/serialdefinition.h \
#        serialcom/serialprotocol.h \
#        serialcom/serialprotocol3.h \
#        serialcom/serialprotocolabstract.h \
#        serialcom/stmhidport.h \
#        serialcom/stmhidtester.h

#buildtimeTarget.target = builddatetime.h
#buildtimeTarget.depends = FORCE
#buildtimeTarget.commands = touch $$PWD/builddatetime.h
#PRE_TARGETDEPS += builddatetime.h

QMAKE_EXTRA_TARGETS += buildtimeTarget

FORMS += \
        mainwindow.ui

INCLUDEPATH +=/opt/qt5pi/sysroot/usr/include

#LIBS += -L/opt/qt5pi/sysroot/usr/lib -lwiringPi \
LIBS += -L/opt/qt5rpi3/sysroot/usr/lib -lhidapi-hidraw \
        -L/opt/qt5rpi3/sysroot/usr/local/qt5pi/lib -lQt5Network \
        -L/opt/qt5rpi3/sysroot/usr/local/qt5pi/lib -lQt5SerialPort

QMAKE_CFLAGS_ISYSTEM = -I

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/pi/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
