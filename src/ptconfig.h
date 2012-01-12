#ifndef ptCONFIG_H_
#define ptCONFIG_H_

/** @file ptconfig.h
    picotype library compile configuration.

    This file is a default configuration. To customize picotype for your project
    copy this file to your project directory and modify as needed. Be sure to add 
    the path to the modified ptconfig.h to picotypes compiler include path, before
    picotype dir (picotype/).
*/

/** Shift factor for conversion between an X-coordinate in pixels and units. */
#define ptGCEIGHTX 0
/** Shift factor for conversion between an Y-coordinate in pixels and units. */
#define ptGCEIGHTY 0

/** Compile support for 16 bpp graphics formats (e.g. with ptSprite). */
#define ptUSE_16BPP 1

/** Option for #pt16BPP, %0bbbbbgggggrrrrr. Do not change. */
#define pt16BPP_BGR555 0
/** Option for #pt16BPP, %rrrrrggggggbbbbb. Do not change. */
#define pt16BPP_RGB565 1

/** Sets 16 bpp RGB bitfield layout. */
#if bbOS == bbOS_PALMOS
#define pt16BPP pt16BPP_RGB565
#elif bbOS == bbOS_RISCOS
#define pt16BPP pt16BPP_BGR555
#else
/* change here if needed */
#define pt16BPP pt16BPP_BGR555
#endif

/** Compile support for ptGC8 8 bpp palette-indexed custom graphics context. */
#define ptUSE_GC8 1

/** Compile support for ptGC16 16 bpp true colour custom graphics context. */
#define ptUSE_GC16 1

/** Compile JPEG support with ptImage_jpeglib via jpeglib. */
#define ptUSE_JPEGLIB 0

/** ptTextBox::pLines is list of bbHEAPH instead list of pointers. */
#define ptTEXTBOX_HEAPH 1

/** Compile support for ZapFonts in ptFont_mem. */
#define ptUSE_FONTZAP 1

/** Compile support for BBC Fonts in ptFont_mem. */
#define ptUSE_FONTBBC 1

/** Compile support for Windows fonts via ptFont_win32. */
#if !defined(ptUSE_FONTWIN) && !defined(bbQT)
#define ptUSE_FONTWIN 1
#endif

/** Compile support for Windows GDI via ptGCGDI. */
#if bbOS == bbOS_WIN32
#define ptUSE_GCGDI 1
#endif

/** Compile support for QT QPainter via ptGCQT. */
#define ptUSE_GCQT 1

/** Version of this ptconfig.h file, do not change.
     - V2 : added ptTEXTBOX_HEAPH
     - V3 : added ptUSE_FONTZAP, ptUSE_FONTWIN
     - V4 : added ptUSE_FONTBBC
*/
#define ptCONFIGVER 4

#endif /* ptCONFIG_H_ */
