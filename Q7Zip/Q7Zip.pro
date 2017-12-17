#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T11:23:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Q7Zip
TEMPLATE = app

CONFIG(debug, debug|release){
    DEFINES += DEBUG_LOGOUT_ON
    message("Debug Build")

#    DEFINES += VLD_ENABLED

    contains( DEFINES, VLD_ENABLED ) {
        VLD_PATH = $$PWD/../vld-2.5.1
        INCLUDEPATH += $$VLD_PATH/include
        LIBS += -L$$VLD_PATH/lib/Win32 -lvld
        LIBS += -L$$VLD_PATH/bin/Win32
    }
}

CONFIG(release, debug|release){
    message("Release Build")
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# For LZMA SDK 7z library (7zra.lib/7zra.dll)
DEFINES += _UNICODE

INCLUDEPATH += $$PWD/7z_sdk/include

LIBS        += -L$$PWD/7z_sdk/lib
LIBS        += 7zra.lib


SOURCES += \
    main.cpp \
    q7zip_window.cpp \
    q7zip.cpp \
    7z_sdk/src/FileStreams.cpp

HEADERS  += \
    q7zip_window.h \
    q7zip.h

# For LZMA SDK Source Code
SOURCES += \
    7z_sdk/src/FileIO.cpp

HEADERS  += \
    7z_sdk/include/7zTypes.h \
    7z_sdk/include/Common.h \
    7z_sdk/include/Compiler.h \
    7z_sdk/include/Defs.h \
    7z_sdk/include/FileIO.h \
    7z_sdk/include/FileName.h \
    7z_sdk/include/FileStreams.h \
    7z_sdk/include/IArchive.h \
    7z_sdk/include/IDecl.h \
    7z_sdk/include/IProgress.h \
    7z_sdk/include/IStream.h \
    7z_sdk/include/MyBuffer.h \
    7z_sdk/include/MyCom.h \
    7z_sdk/include/MyString.h \
    7z_sdk/include/MyTypes.h \
    7z_sdk/include/MyUnknown.h \
    7z_sdk/include/MyVector.h \
    7z_sdk/include/MyWindows.h \
    7z_sdk/include/PropID.h \
    7z_sdk/include/StdAfx.h

FORMS    += \
    q7zip_window.ui

RESOURCES   += \
    image.qrc

RC_FILE     += \
    Q7Zip.rc

DISTFILES   += \
    Q7Zip.rc
