TARGET = windisk
TEMPLATE = app
QT += quick core qml quickcontrols2

CONFIG += c++11

VERSION = 1.0.2.0

DEFINES += VERSION_NUMBER=\\\"$${VERSION}\\\"
win32 {
    DEFINES += VERSION_RC=$$replace(VERSION,"\.",",")
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += WINVER=0x0601
DEFINES += _WIN32_WINNT=0x0601
DEFINES -= UNICODE

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    deviceitem.cpp \
    guimanager.cpp \
    diskutilities.cpp \
    winnativeeventfilter.cpp \
    deviceevent.cpp

RESOURCES += qml.qrc \
    images.qrc

OTHER_FILES += windisk.rc

win32 {
    RC_FILE = windisk.rc
    QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
}
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(../qmlmodel/qmlmodel.pri)

HEADERS += \
    deviceitem.h \
    guimanager.h \
    diskutilities.h \
    winnativeeventfilter.h \
    deviceevent.h \
    sysdef.h
