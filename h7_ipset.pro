QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    pc_protocol.c \
    protocol/protocol_frame.cpp \
    communication/serial_thread.cpp \
    communication/socket_thread.cpp \
    ui/config_widget.cpp \
    ui/debug_widget.cpp

HEADERS += \
    mainwindow.h \
    pc_protocol.h \
    protocol/protocol_frame.h \
    communication/serial_thread.h \
    communication/socket_thread.h \
    ui/config_widget.h \
    ui/debug_widget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
