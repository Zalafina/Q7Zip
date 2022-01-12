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

#    DEFINES += VLD_ENABLED

    contains( DEFINES, VLD_ENABLED ) {
        VLD_PATH = $$PWD/../vld-2.5.1
        INCLUDEPATH += $$VLD_PATH/include
        contains(DEFINES, WIN64) {
            message("WIN64 Debug Build")
            LIBS += -L$$VLD_PATH/lib/Win64 -lvld
            LIBS += -L$$VLD_PATH/bin/Win64
        } else {
            message("WIN32 Debug Build")
            LIBS += -L$$VLD_PATH/lib/Win32 -lvld
            LIBS += -L$$VLD_PATH/bin/Win32
        }
    }
}

CONFIG(release, debug|release){
    contains(DEFINES, WIN64) {
        message("WIN64 Release Build")
    } else {
        message("WIN32 Release Build")
    }
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

INCLUDEPATH += $$PWD/7z_sdk

contains(DEFINES, WIN64) {
# Win x64 libs
LIBS        += -L$$PWD/win_lib/x64
} else {
# Win x86 libs
LIBS        += -L$$PWD/win_lib/x86
}
LIBS        += OleAut32.lib User32.Lib


SOURCES += \
    main.cpp \
    q7zip_window.cpp \
    q7zip.cpp \
    mfiledialog.cpp \
    progress_window.cpp

HEADERS  += \
    q7zip_window.h \
    q7zip.h \
    mfiledialog.h \
    progress_window.h

# For LZMA SDK Source Code
SOURCES += \
    7z_sdk/Windows/FileDir.cpp \
    7z_sdk/Windows/FileFind.cpp \
    7z_sdk/Windows/FileIO.cpp \
    7z_sdk/Windows/FileName.cpp \
    7z_sdk/Windows/PropVariant.cpp \
    7z_sdk/Windows/PropVariantConv.cpp \
    7z_sdk/Common/MyWindows.cpp \
    7z_sdk/Common/MyString.cpp \
    7z_sdk/7Zip/Common/FileStreams.cpp \
    7z_sdk/Common/IntToString.cpp \
    7z_sdk/Common/StringConvert.cpp

HEADERS  += \
    7z_sdk/Windows/Defs.h \
    7z_sdk/Windows/FileIO.h \
    7z_sdk/Windows/FileName.h \
    7z_sdk/Common/Common.h \
    7z_sdk/Common/Defs.h \
    7z_sdk/Common/MyBuffer.h \
    7z_sdk/Common/MyCom.h \
    7z_sdk/Common/MyString.h \
    7z_sdk/Common/MyTypes.h \
    7z_sdk/Common/MyUnknown.h \
    7z_sdk/Common/MyVector.h \
    7z_sdk/Common/MyWindows.h \
    7z_sdk/C/7zVersion.h \
    7z_sdk/C/7zTypes.h \
    7z_sdk/C/Compiler.h \
    7z_sdk/C/CpuArch.h \
    7z_sdk/7zip/IDecl.h \
    7z_sdk/7zip/IProgress.h \
    7z_sdk/7zip/IStream.h \
    7z_sdk/7zip/PropID.h \
    7z_sdk/7zip/Archive/IArchive.h \
    7z_sdk/7zip/Common/FileStreams.h

FORMS    += \
    q7zip_window.ui \
    progress_window.ui

RESOURCES   += \
    image.qrc

RC_FILE     += \
    Q7Zip.rc

DISTFILES   += \
    Q7Zip.rc
