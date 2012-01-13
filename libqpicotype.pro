TEMPLATE = lib
CONFIG += staticlib
TARGET = libqpicotype
DEPENDPATH += .
INCLUDEPATH += src ../babel/include
DEFINES += bbQT

win32 {
QMAKE_CXXFLAGS += /Zc:wchar_t /Zp4
QMAKE_CXXFLAGS_DEBUG += /Zc:wchar_t /Zp4
}

# Input
HEADERS += \
    src/pt.h \
    src/ptFont.h \
    src/ptFontMan.h \
    src/ptFont_mem.h \
    src/ptFont_win.h \
    src/ptGC.h \
    src/ptGC16.h \
    src/ptGC8.h \
    src/ptGCGDI.h \
    src/ptGCQT.h \
    src/ptImage.h \
    src/ptImage_jpeglib.h \
    src/ptPal.h \
    src/ptPalMan.h \
    src/ptSprite.h \
    src/ptcol.h \
    src/ptconfig.h \
    src/ptconvert.h \
    src/ptdefs.h \

SOURCES += \
    src/ptCol.cpp \
    src/ptFont.cpp \
    src/ptFontMan.cpp \
    src/ptFont_mem.cpp \
    src/ptFont_win.cpp \
    src/ptGC.cpp \
    src/ptGC16.cpp \
    src/ptGC8.cpp \
    src/ptGCGDI.cpp \
    src/ptGCQT.cpp \
    src/ptImage.cpp \
    src/ptImage_jpeglib.cpp \
    src/ptPal.cpp \
    src/ptPalMan.cpp \
    src/ptSprite.cpp \
    src/ptconvert.cpp \
