#############################################################################
# Makefile for building: libqpicotype.a
# Generated by qmake (2.01a) (Qt 4.8.0) on: Wed Mar 14 10:44:37 2012
# Project:  libqpicotype.pro
# Template: lib
# Command: /usr/local/Trolltech/Qt-4.8.0/bin/qmake -o Makefile libqpicotype.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DbbQT -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -m64 -pipe -O2 -fPIC -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -m64 -pipe -O2 -fPIC -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/local/Trolltech/Qt-4.8.0/mkspecs/linux-g++-64 -I. -I/usr/local/Trolltech/Qt-4.8.0/include/QtCore -I/usr/local/Trolltech/Qt-4.8.0/include/QtGui -I/usr/local/Trolltech/Qt-4.8.0/include -Isrc -I../babel/include -I.
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/local/Trolltech/Qt-4.8.0/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = src/ptCol.cpp \
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
		src/ptconvert.cpp 
OBJECTS       = ptCol.o \
		ptFont.o \
		ptFontMan.o \
		ptFont_mem.o \
		ptFont_win.o \
		ptGC.o \
		ptGC16.o \
		ptGC8.o \
		ptGCGDI.o \
		ptGCQT.o \
		ptImage.o \
		ptImage_jpeglib.o \
		ptPal.o \
		ptPalMan.o \
		ptSprite.o \
		ptconvert.o
DIST          = /usr/local/Trolltech/Qt-4.8.0/mkspecs/common/unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/linux.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base-unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-base.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/qconfig.pri \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_functions.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_config.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/exclusive_builds.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_pre.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/release.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_post.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/staticlib.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/static.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/warn_on.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/thread.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/moc.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/resources.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/uic.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/yacc.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/lex.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/include_source_dir.prf \
		libqpicotype.pro
QMAKE_TARGET  = qpicotype
DESTDIR       = 
TARGET        = libqpicotype.a

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET) 

staticlib: $(TARGET)

$(TARGET):  $(OBJECTS) $(OBJCOMP) 
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(OBJECTS)


Makefile: libqpicotype.pro  /usr/local/Trolltech/Qt-4.8.0/mkspecs/linux-g++-64/qmake.conf /usr/local/Trolltech/Qt-4.8.0/mkspecs/common/unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/linux.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base-unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-base.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-unix.conf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/qconfig.pri \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_functions.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_config.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/exclusive_builds.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_pre.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/release.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_post.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/staticlib.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/static.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/warn_on.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/thread.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/moc.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/resources.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/uic.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/yacc.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/lex.prf \
		/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/include_source_dir.prf \
		/usr/local/Trolltech/Qt-4.8.0/lib/libQtGui.prl \
		/usr/local/Trolltech/Qt-4.8.0/lib/libQtCore.prl
	$(QMAKE) -o Makefile libqpicotype.pro
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/unix.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/linux.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/gcc-base-unix.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-base.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/common/g++-unix.conf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/qconfig.pri:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_functions.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt_config.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/exclusive_builds.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_pre.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/release.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/default_post.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/staticlib.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/static.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/gdb_dwarf_index.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/warn_on.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/qt.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/unix/thread.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/moc.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/resources.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/uic.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/yacc.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/lex.prf:
/usr/local/Trolltech/Qt-4.8.0/mkspecs/features/include_source_dir.prf:
/usr/local/Trolltech/Qt-4.8.0/lib/libQtGui.prl:
/usr/local/Trolltech/Qt-4.8.0/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -o Makefile libqpicotype.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/qpicotype1.0.0 || $(MKDIR) .tmp/qpicotype1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/qpicotype1.0.0/ && $(COPY_FILE) --parents src/pt.h src/ptFont.h src/ptFontMan.h src/ptFont_mem.h src/ptFont_win.h src/ptGC.h src/ptGC16.h src/ptGC8.h src/ptGCGDI.h src/ptGCQT.h src/ptImage.h src/ptImage_jpeglib.h src/ptPal.h src/ptPalMan.h src/ptSprite.h src/ptCol.h src/ptconfig.h src/ptconvert.h src/ptdefs.h .tmp/qpicotype1.0.0/ && $(COPY_FILE) --parents src/ptCol.cpp src/ptFont.cpp src/ptFontMan.cpp src/ptFont_mem.cpp src/ptFont_win.cpp src/ptGC.cpp src/ptGC16.cpp src/ptGC8.cpp src/ptGCGDI.cpp src/ptGCQT.cpp src/ptImage.cpp src/ptImage_jpeglib.cpp src/ptPal.cpp src/ptPalMan.cpp src/ptSprite.cpp src/ptconvert.cpp .tmp/qpicotype1.0.0/ && (cd `dirname .tmp/qpicotype1.0.0` && $(TAR) qpicotype1.0.0.tar qpicotype1.0.0 && $(COMPRESS) qpicotype1.0.0.tar) && $(MOVE) `dirname .tmp/qpicotype1.0.0`/qpicotype1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/qpicotype1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all:
compiler_moc_header_clean:
compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: 

####### Compile

ptCol.o: src/ptCol.cpp src/ptCol.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPal.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptCol.o src/ptCol.cpp

ptFont.o: src/ptFont.cpp src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptFont.o src/ptFont.cpp

ptFontMan.o: src/ptFontMan.cpp src/ptFontMan.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptFont.h \
		src/ptFont_mem.h \
		src/ptFont_win.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptFontMan.o src/ptFontMan.cpp

ptFont_mem.o: src/ptFont_mem.cpp src/ptFont_mem.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptFont_mem.o src/ptFont_mem.cpp

ptFont_win.o: src/ptFont_win.cpp src/ptFont_win.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptFont_win.o src/ptFont_win.cpp

ptGC.o: src/ptGC.cpp src/ptGC.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptFontMan.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptGC.o src/ptGC.cpp

ptGC16.o: src/ptGC16.cpp src/ptGC16.h \
		src/ptGC.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptSprite.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptGC16.o src/ptGC16.cpp

ptGC8.o: src/ptGC8.cpp src/ptGC8.h \
		src/ptGC.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptFontMan.h \
		src/ptSprite.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptGC8.o src/ptGC8.cpp

ptGCGDI.o: src/ptGCGDI.cpp src/ptGCGDI.h \
		src/ptGC.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptFontMan.h \
		src/ptSprite.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptGCGDI.o src/ptGCGDI.cpp

ptGCQT.o: src/ptGCQT.cpp src/ptGCQT.h \
		src/ptGC.h \
		src/ptFont.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptSprite.h \
		src/ptconvert.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptGCQT.o src/ptGCQT.cpp

ptImage.o: src/ptImage.cpp src/ptImage.h \
		src/ptSprite.h \
		src/ptdefs.h \
		src/ptconfig.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptImage.o src/ptImage.cpp

ptImage_jpeglib.o: src/ptImage_jpeglib.cpp src/ptImage_jpeglib.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptImage.h \
		src/ptSprite.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptImage_jpeglib.o src/ptImage_jpeglib.cpp

ptPal.o: src/ptPal.cpp src/ptPal.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptCol.h \
		src/ptGC.h \
		src/ptFont.h \
		src/ptPalMan.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptPal.o src/ptPal.cpp

ptPalMan.o: src/ptPalMan.cpp src/ptPalMan.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPal.h \
		src/ptCol.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptPalMan.o src/ptPalMan.cpp

ptSprite.o: src/ptSprite.cpp src/ptSprite.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPalMan.h \
		src/ptPal.h \
		src/ptCol.h \
		src/ptconvert.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptSprite.o src/ptSprite.cpp

ptconvert.o: src/ptconvert.cpp src/ptconvert.h \
		src/ptdefs.h \
		src/ptconfig.h \
		src/ptPal.h \
		src/ptCol.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ptconvert.o src/ptconvert.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

