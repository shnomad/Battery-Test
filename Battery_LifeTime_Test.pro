#-------------------------------------------------
#
# Project created by QtCreator 2019-08-30T15:14:28
#
#-------------------------------------------------

QT       += core gui
QT       += network

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
        gpiocontrol.cpp \
        hid_uart_comm.cpp \
        main.cpp \
        mainwindow.cpp \
        measurement.cpp \
        relay_seed_ddl.cpp \
        settings.cpp \
        loggingcategories.cpp \
        sys_cmd_resp.cpp \
#       tcpsocketrw.cpp \
#       serialcom/hidtester.cpp \
#       serialcom/serialcomm.cpp \
#       serialcom/serialporttester.cpp \
#       serialcom/serialprotocol1.cpp \
#       serialcom/serialprotocol2.cpp \
#       serialcom/serialprotocol3.cpp \
        serialcom/serialprotocolabstract.cpp
#       serialcom/stmhidport.cpp
#       serialcom/stmhidtester.cpp

HEADERS += \
        bgms_set_param_info.h \
        control.h \
        gpiocontrol.h \
        hid_uart_comm.h \
        mainwindow.h \
        measurement.h \
        python_wrapper.h \
        relay_seed_ddl.h \
        commondefinition.h \
        settings.h \
        setting_flagname_definition.h \
        builddatetime.h \
        loggingcategories.h \
        measurement_param.h \
        sys_cmd_resp.h \
#       python_wrapper.h
#       tcpsocketrw.h
#       serialcom/glucosedownloadprogress.h \
#       serialcom/hidtester.h \
#       serialcom/serialcomm.h \
#       serialcom/serialdefinition.h \
#       serialcom/serialporttester.h \
        serialcom/serialprotocol.h \
#       serialcom/serialprotocol1.h \
#       serialcom/serialprotocol2.h \
#       serialcom/serialprotocol3.h \
        serialcom/serialprotocolabstract.h
#       serialcom/glucosedownloadprogress.h \
#        serialcom/stmhidport.h
#       serialcom/stmhidtester.h

#buildtimeTarget.target = builddatetime.h
#buildtimeTarget.depends = FORCE
#buildtimeTarget.commands = touch $$PWD/builddatetime.h
#PRE_TARGETDEPS += builddatetime.h

QMAKE_EXTRA_TARGETS += buildtimeTarget

FORMS += \
        mainwindow.ui

INCLUDEPATH +=/opt/qt5pi/sysroot/usr/include

#LIBS += -L/opt/qt5pi/sysroot/usr/lib -lwiringPi \
LIBS += -L/opt/qt5rpi3/sysroot/usr/lib -lhidapi-hidraw
#        -L/opt/qt5rpi3/sysroot/usr/lib/python3.7/config-3.7m-arm-linux-gnueabihf -lpython3.7
#        -L/opt/qt5rpi3/sysroot/usr/local/qt5pi/lib -lQt5Network \
#        -L/opt/qt5rpi3/sysroot/usr/local/qt5pi/lib -lQt5SerialPort

QMAKE_CFLAGS_ISYSTEM = -I

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/pi/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#RESOURCES += \
#    resource.qrc
