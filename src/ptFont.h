#ifndef ptFONT_H_
#define ptFONT_H_

/** @file
*/

#include <babel/defs.h>
#include <babel/file.h>
#include <babel/Arr.h>

#include "ptdefs.h"

/** ptFont: Log2 of size of an internal codepoint bank */
#define ptFONT_BANKSIZE_LOG2 7
/** ptFont: Size of an internal codepoint bank */
#define ptFONT_BANKSIZE 128
/** ptFont: Log2 of number of internal codepoint banks per Unicode page */
#define ptFONT_BANKCOUNT_LOG2 9
/** ptFont: Number of internal codepoint banks per Unicode page */
#define ptFONT_BANKCOUNT 512
/** ptFont: Maximum width, constrained by 6 bit field in bank index. Must be pwr2-1. */
#define ptFONT_MAXWIDTH 63
/** ptFont: Maximum height, constrained by ptFont::mChHeight bitwidth. Must be pwr2-1. */
#define ptFONT_MAXHEIGHT 255

enum ptFONTTYPE
{
    ptFONTTYPE_NONE = 0,
    ptFONTTYPE_MEM,
    ptFONTTYPE_WIN,
    ptFONTTYPE_COMPOSITE,
    ptFONTTYPE_DEFAULT = 255,
};

/** Font format IDs, see ptFont::mFormat. */
enum ptFONTFORMAT
{
    ptFONTFORMAT_ZAP = 0,       //!< !Zap bitmap fonts
    ptFONTFORMAT_BBC,           //!< Acorn BBC bitmap fonts
    ptFONTFORMAT_JWP,           //!< JWP bitmap fonts
    ptFONTFORMAT_WINRASTER,     //!< Windows bitmap fonts
    ptFONTFORMAT_WINVECTOR,     //!< Windows vector fonts
    ptFONTFORMAT_WINTRUETYPE,   //!< Windows TrueType and OpenType fonts
    ptFONTFORMATCOUNT,          //!< Number of known font formats
    ptFONTFORMATALL = -1        //!< Special constant meaning any font
};

#define ptFONTFORMATNAMES \
    bbT("ZapFont"), \
    bbT("BBCFont"), \
    bbT("JWP"), \
    bbT("Raster"), \
    bbT("Vector"), \
    bbT("TrueType")

enum ptFONTSTYLE
{
    ptFONTSTYLE_REGULAR    = 0,
    ptFONTSTYLE_ITALIC     = 1,
    ptFONTSTYLE_BOLD       = 2,
    ptFONTSTYLE_BOLDITALIC = 3,
    ptFONTSTYLECOUNT // must be pwr2
};

/** Font descriptor.
    Container for font meta-info, used to create and describe ptFont instances.
    If this structure is used to create a font via ptFontMan::GetFont(), a subset
    of its members need to be set, depending on ptFontInfo::mType and ptFontInfo::mFormat:
    <pre>

    mType                Required as input

    ptFONTTYPE_MEM       mpFont
    ptFONTTYPE_WIN       mpFont, mFormat, mWidth, mHeight, mStyle, mCharset
    ptFONTTYPE_COMPOSITE not supported yet    

    </pre>
    All other types take no input parameters create the default font.
*/
struct ptFontInfo
{
    bbU8        mType;      //!< ptFONTTYPE
    bbU8        mFormat;    //!< ptFONTFORMAT
    bbU8        mWidth;     //!< Character width, 0 if freely scalable, only valid for ptFONTTYPE_WIN
    bbU8        mHeight;    //!< Character height, only valid for ptFONTTYPE_WIN
    bbU8        mStyle;     //!< ptFONTSTYLE, only valid for ptFONTTYPE_WIN
    bbU8        mCharset;   //!< GDI charset, only valid for ptFONTTYPE_WIN
    bbU8        mMonospace; //!< true, if monospace font
    bbU8        mCodepage;  //!< bbCPG font codepage
    bbCHAR*     mpName;     //!< Display name, pointer to heap block
    bbCHAR*     mpFont;     //!< Font Allocation Name, pointer to heap block, can be same block as pFont or different block

    inline void Init() { bbMemClear(this, sizeof(ptFontInfo)); }
    inline void Destroy() { Clear(); }
    void        Clear();
    bbERR       CopyTo(ptFontInfo* const pTo) const;
    bbCHAR*     ToStr();
    bbERR       FromStr(const bbCHAR* pStr);
};

/** Describes a font glyph for ptFont.
    If ptFontGlyph::opt ptFONTGLYPH_OPT_UKCP is set, the rest of the
    structure descibes the glyph for the replacement codepoint ptFont::mUkCP.
*/
struct ptFontGlyph
{
    const bbU8* pBmp;   //!< Pointer to bitmap for glyph
    bbU8        width;  //!< Width of glyph in pixels
    bbU8        opt;    //!< Glyph properties, see ptFONTGLYPH_OPT_*

};

/** ptFontGlyph::opt flagbit: Codepoint is unknown to font. */
#define ptFONTGLYPH_OPT_UKCP 0x1U

/** Font interface.

    This font class supports bitmap fonts with up to 0x110000 glyphs.
    Each glyph is addressed by a codepoint, which is a number of type bbCHARCP.
    For correct character code page mapping the font must have the same code page
    as the default code page #bbCPG.

    ptFont handles glyphs as equivalent to codepoints.

    Glyph bitmaps are cached via the following hierarchy of data structures:
     - mpPages[] addresses up to 17 pages of 65536 code points each. If the bbCHARCP type can hold
       only 16 bits or less, this array will be only of size 1. Each entry of mpPages[] is a pointer
       to a page. Pointer is 0 if page not existent, 1 if existent but not loaded.
     - A page is an array of #ptFONT_BANKCOUNT pointers to banks,
       pointer is 0 if bank not existent, 1 if existent but not loaded.
     - A bank is an array of #ptFONT_BANKSIZE 32 bit words, resembling the bank index,
       followed by an array of #ptFONT_BANKSIZE glyph bitmaps
     - A 32 bit array entry has the following format:
       <pre>
       0 if glyph not existent, or
       bit  0-23 : Offset to glyph bitmap
       bit 24-25 : reserved
       bit 26-31 : Glyph width in pixels
       </pre>

     ptFont can handle monospace and variable width characters. Variable width character fonts are
     identified by setting ptFont::mChWidth to 0. For monospace fonts, ptFont::mChWidth designates
     the width of normal width characters. Double width (widechar) fonts are of width ptFont::mChWidth * 2.

     Unknown codepoints in variable width fonts are replaced by the default codepoint ptFont::mUkCP.
     For monospace fonts, unknown normal width characters are replaced by ptFont::mUkCP and unknown
     double width are replaced by ptFont::mUkCPW. ptFont::mUkCPW is unused for variable width fonts.
*/
class ptFont
{
public:
    ptFontInfo  mInfo;      //!< Font info
    bbU32       mRefCount;  //!< Reference count
    bbCHARCP    mUkCP;      //!< Code point to use for normal width code points unknown to the font.
    bbCHARCP    mUkCPW;     //!< Code point to use for double width code points unknown to the font.
    const bbU8* mpBankUkCP; //!< Pointer to bank of ptFont::mUkCP.
    const bbU8* mpBankUkCPW;//!< Pointer to bank of ptFont::mUkCPW.

protected:
    /** Pointers for bank indices of pages of 65536 codepoints each.
        If bbCHARCP is larger than 16 bits, this array can address 16 pages,
        otherwise 1 page. Semantic for array entries:
         - 0 if bank not existent,
         - 1 if bank existent but not loaded,
         - Pointer otherwise.
    */
    #if (bbSIZEOF_CHARCP > 2)
    const bbU8* const * mpPages[17];
    #else
    const bbU8* const * mpPages[1];
    #endif

    /** Set replacement code point to use for normal/variable width code points unknown to the font.
        The font object must be initialized with a font.
        This codepoint will also be used, if not enough memory is available for
        processing uncached code points. The call may fail due to out of memory.
        @param cp Replacement code point. Must exist in font.
        @return bbEOK on success, or value of #bbgErr on failure.
        @retval bbEOK Success
        @retval bbEUK Code point \a cp is unknown by this font and cannot be used
        @retval bbENOMEM Out of memory
    */
    bbERR SetUnknownCP(const bbCHARCP cp);

    /** Set replacement code point to use for double width code points unknown to the font.
        @see ptFont::SetUnknownCP() for more info.
    */
    bbERR SetUnknownCPW(const bbCHARCP cp);

    /** Enforce codepoint to be allocated, and get pointer to its bitmap.
        @param cp    Codepoint
        @param width Width of bitmap in pixels, used only if codepoint is not existent in font.
        @return Pointer to bitmap, or NULL on error
    */
    bbU8* EditCP(const bbCHARCP cp, const bbUINT width);

    /** Load page the given code point lies within.
        If loading the page fails, NULL is returned.
        @param cp Code point
        @return Pointer to loaded page or NULL on error
    */
    virtual const bbU8* const* LoadPage(bbCHARCP const cp);

    /** Load code point bank.
        The code point's page must be loaded already.
        If loading the bank fails, NULL is returned.
        @param cp Code point
        @return Pointer to loaded bank or NULL on error
    */
    virtual const bbU8* LoadBank(bbCHARCP const cp);

#if ptUSE_GC8 == 1
    friend class ptGC8;
#endif
#if ptUSE_GC16 == 1
    friend class ptGC16;
#endif

public:
    /** Get bank for given code point, do not replace if unknown.
        If the bank is not loaded yet, it will be loaded.
        @param cp Codepoint
        @return Pointer to bank, or NULL if codepoint unknown (bbErrGet() is bbEUK) or error
    */
    const bbU8* GetBankNoReplace(bbCHARCP const cp);

public:
    ptFont();
    virtual ~ptFont();

    /** Clear font container.
        This function will detach opened files and clear all memory associated with
        the opened font.

        The default implementation performs the following actions:
         - free heap block with font name in mpName
         - free heap blocks for all loaded font banks referenced via mpPages[]
         - free heap blocks for all loaded font pages in mpPages[]
         - reset unknown code point mUkCP to 0
    */
    virtual void Clear();

    inline bbUINT GetHeight() const
    {
        return (bbUINT) mInfo.mHeight;
    }

    /** Get font width.
        For monospace fonts this is each character's width.
        For monospace fonts that include double width characters, this is the half width's character's width.
        For variably spaced fonts, this is 0.
    */
    inline bbUINT GetWidth() const
    {
        return (bbUINT) mInfo.mWidth;
    }

    /** Get unknown codepoint for variable width/normal width characters.
        @return Replacement codepoint
    */
    inline bbCHARCP GetUkCP() const
    {
        return mUkCP;
    }

    /** Get unknown codepoint for double width characters.
        @return Replacement codepoint
    */
    inline bbCHARCP GetUkCPW() const
    {
        return mUkCPW;
    }

    /** Get codepage. */
    inline bbUINT GetCPG() const
    {
        return (bbUINT) mInfo.mCodepage;
    }

    /** Get font format.
        @return Format ID
    */
    inline ptFONTFORMAT GetFormat() const
    {
        return (ptFONTFORMAT)mInfo.mFormat;
    }

    bbERR GetInfo(ptFontInfo* const pInfo);
    
    /** Lookup and cache glyph array for a string.
        The returned bitmap pointers are valid only as long as no other ptFont functions are called.
        @param pText   Pointer to string in encoding specified by #bbENC.
                       Must be well-formed and does not need to be 0-terminated.
        @param len     Length of string in code units
        @param pGlyphs Pointer to buffer to be filled with glyph data.
                       Must be of size \a len (\a len * sizeof(ptFontGlyph) bytes).
        @return Number of glyphs written to \a pGlyphs
    */
    //bbUINT PrepareText(const bbCHAR* const pText, const bbUINT len, ptFontGlyph* const pGlyphs);

    /** Get glyph for codepoint.
        The returned bitmap pointer is valid only as long as no other ptFont functions are called.
        @param cp     Character codepoint
        @param pGlyph Will be filled in with bitmap pointer and glyph info
    */
    void GetGlyph(bbCHARCP cp, ptFontGlyph* const pGlyph);
};

bbDECLAREARRPTR(ptFont*, ptArrPFont);

#endif /* ptFONT_H_ */

