#ifndef ptFONT_WIN_H_
#define ptFONT_WIN_H_

/** @file
*/

#include "ptFont.h"

#if ptUSE_FONTWIN == 1

/** ptFont_win::Init() flagbit: Lock font's width-height aspect ratio. */
#define ptFONT_WIN_INIT_NOHSCALE 0x1

/** Font implementation which maps Windows fonts to picotype.

    All windows font will be mapped as monospace fonts, taking
    Unicode half- and fullwidth character codepoints into account.
    Halfwidth characters are rendered in the basis width of the font,
    fullwidth characters are rendered with twice the basis width.
*/
class ptFont_win : public ptFont
{
protected:
    HDC     mhDC;
    HFONT   mhFont;
    HBITMAP mhBmp;

    virtual const bbU8* const* LoadPage(bbCHARCP const cp);
    virtual const bbU8* LoadBank(bbCHARCP const cp);

    /** Init array available of available pages/banks.
        @param pPage    Pointer to page (array of banks), or NULL to init mPages[]
        @param pFirstCP if pPage NULL, then this returns available codepoint,
                        otherwise this is an int with the page number for which to get the bank index
    */
    bbERR GetAvailableCP(const bbU8** pPage, bbCHARCP* const pFirstCP);

public:
    ptFont_win();
    ~ptFont_win();// workaround for VC6 bug

    virtual void Clear();

    /** Init specified windows font.
        @param pName   Typeface name, 0-terminated.
        @param width   Font width
        @param height  Font height
        @param style   Font style
        @param charset GDI character set ID
        @param opt     Flagbits:
                       - ptFONT_WIN_INIT_NOHSCALE -- Lock font's width-height aspect ratio,
                         i.e. do not scale width to fit parameter \a width. Usefull for Windows
                         raster fonts, since they luck ugly when scaled.
    */
    bbERR Init(ptFONTFORMAT const format, const bbCHAR* const pName, bbUINT const width, bbUINT const height, ptFONTSTYLE const style, bbUINT const charset, bbU32 const opt);
};

#endif /* ptUSE_FONTWIN */
#endif /* ptFONT_WIN_H_ */
