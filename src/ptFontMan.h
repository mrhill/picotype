#ifndef ptFONTMAN_H_
#define ptFONTMAN_H_

/** @file
*/

#include <babel/defs.h>
#include <babel/Arr.h>

#include "ptdefs.h"
#include "ptFont.h"

class ptFontMan
{
public:
    ptArrPFont  mFonts;         /**< Registry for font objects. */
    bbCHAR*     mpFontDir;      /**< Font directory, bbDIRDELIM + 0-terminated. NULL if no dir is set (default). */
    bbUINT      mhDefaultFont;  /**< Default font handle. */

private:
    /** Add font object to registry.
        @param pFont Font object
        @return Runtime font handle, or -1 on error
    */
    bbUINT AddFont(ptFont* const pFont);
public:

    /** Init font manager.
    */
    bbERR Init();

    /** Destroy font manager. */
    void Destroy();

    /** Set font directory.
        @param pDir 0-terminated path string, will be copied and optionally appended with bbDIRDELIM
    */
    bbERR SetFontDir(const bbCHAR* const pDir);

    /** Get reference counted font object.
        If a font object was returned successfully, it must be
        released later by calling ptFontMan::ReleaseFont().
        May reallocate fontpool ptFontMan::mFonts.
        @param pInfo Font description
                     Struct members are valid depending on pInfo.mType:
                     - ptFONTTYPE_DEFAULT : none
                     - ptFONTTYPE_MEM : mpFont, 
                     - ptFONTTYPE_WIN : mpFont,  mWidth, mHeight, mStyle, mCharset, ptFONT_WIN_INIT_NOHSCALE
                     Can be NULL to get the default font, which always succeeds.
        @return Font handle, use as index into ptFontMan::mFonts, or (bbUINT)-1 on error
    */
    bbUINT GetFont(const ptFontInfo* const pInfo);

    /** Release reference counted font object.
        @param hFont Font handle, can be (bbUINT)-1
    */
    void ReleaseFont(bbUINT const hFont);
};

/** Global font manager instance. */
extern ptFontMan ptgFontMan;

#endif /* ptFONTMAN_H_ */

