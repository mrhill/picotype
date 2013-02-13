#ifndef ptGC_H_
#define ptGC_H_

/** @file
    Interface definition for graphics context.
*/

#include <babel/defs.h>
#include <babel/heap.h>
#include "ptFont.h"
#include "ptPalMan.h"
struct ptSprite;

/** Get unnormalized distance of two rgb colours.
    @param a Colour 1, 0x00BBGGRR
    @param b Colour w, 0x00BBGGRR
    @return Distance, unnormalized
*/
bbU32 ptRGBDist(const bbU32 a, const bbU32 b);

bbU32 ptRGBADistA(const bbU32 a, const bbU32 b);

/** Get weighted RGB value.
    @param a 32-bit start colour, 0x00BBGGRR
    @param a 32-bit destination colour, 0x00BBGGRR
    @param weight Weight as 0.16 bit fixpoint value (ie. 0..0x10000 => 0.0..1.0)
    @return 32-bit weighted colour, 0x00BBGGRR
*/
bbU32 ptGetWeightedRGB(const bbU32 a, const bbU32 b, const bbU32 weight);

/** Map RGB colour to closest matching palette entry.
    @param pRGB    Palette, array of 0x00BBGGRR values
    @param palsize Number of entries in palette
    @param rgb     RGB colour to match, 0x00BBGGRR
    @return Closest palette index
*/
bbUINT ptRGBPalMatch(const bbU32* const pRGB, bbUINT const palsize, const bbU32 rgb);

/** Coordinate for use in graphics context #ptGC. */
typedef struct
{
    int x;
    int y;

} ptCoord;

/** Pen to use with graphics context ptGC member functions.
    Value format (low 16 bit used), for meaning of bit fields
    see table below:
    <pre>
    %ooopppcccccccccc
     |   | |
     |   | 10 bit (bit 0-9) Pen option 1, must be set to 0 if not used
     |   3 bit (bit 10-12) Pen number
     3 bit (bit 13-15) Pen option 2, must be set to 0 if not used

    Pen type           Pen number            Pen option 1        Pen option 2
    Solid colour       ptPEN_COL or 0        logical col number  not used (0)
    Transparent colour ptPEN_TRANS           logical col number  transparency grade (0..7 maps to 0%..100% transparency)
    Fill pattern       ptPEN_PAT             pattern number      not used (0)
    Raster Operation   ptPEN_ROP             logical col number  ROP code, see ptROP

    </pre>
*/
typedef bbUINT ptPEN;

/** Pen for solid colour, see ptPEN. */
#define ptPEN_COL 0U
/** Pen for transparent colour, see ptPEN. */
#define ptPEN_TRANS 0x0400U
/** Pen for solid colour with anti-aliasing, see ptPEN. */
#define ptPEN_AA 0x0800U
/** Pen for fill pattern, see ptPEN. */
#define ptPEN_PAT 0x0C00U
/** Pen for ROP, see ptPEN. */
#define ptPEN_ROP 0x1000U

/** Bitmask for separating colour in a value of type #ptPEN. */
#define ptPENCOLMASK 0x3FFU
/** Bitmask for separating bitfields in a value of type #ptPEN. */
#define ptPENMASK 0xFC00U
/** Bitmask for separating bitfield <i>Pen number</i> in a value of type #ptPEN. */
#define ptPENMASK_TYPE 0x1C00U
/** Bitpos of bitfield <i>Pen option 2</i> in a value of type #ptPEN. */
#define ptPENBITPOS_OPT2 13
/** Bitpos of bitfield <i>Pen number</i> in a value of type #ptPEN. */
#define ptPENBITPOS_TYPE 10

#define ptPENOPT2(o) ((o)<<ptPENBITPOS_OPT2)

enum ptROP
{
    ptROP_NOT = 0,                   //!< Invert pixeldata, regardless of bitdepth or RGB lookup
    ptROP_OR  = 1<<ptPENBITPOS_OPT2, //!< OR colour and pixeldata
};

/** Textbox for use with ptGC::MarkupTextBox.
    A textbox's absolute width and height are implicitely
    defined by the associated textline buffers (\a pLines).
    The other parameters are interpreted as denoted below:
    <pre>
    pLines  virtual textbox & plotbox    screen with plotbox
            +-----------------------+    +------------------------+
    ptr[0]->|ABCDEFGH...            |    |     (x,y)              |
    ptr[1]->| (scrollx,scrolly)     |    |       *---------+      |
    ptr[2]->|         *---------+   | -> |       |  width  |      |
    ptr[3]->|         |  width  |   |    | height|         |      |
    ptr[4]->|   height|         |   |    |       +---------+      |
    ptr[5]->|         +---------+   |    |                        |
    NULL    +-----------------------+    |                        |
                                         +------------------------+
    </pre>
*/
typedef struct
{
    int    unit_x;          //!< X of plotbox on screen in units (top-left of box)
    int    unit_y;          //!< Y of plotbox on screen in units (top-left of box)
    bbUINT unit_width;      //!< Width of plotbox on screen in units
    bbUINT unit_height;     //!< Height of plotbox on screen in units
    int    unit_scrollx;    //!< X scroll position within textbox
    int    unit_scrolly;    //!< Y scroll position within textbox
    bbUINT unit_linespace;  //!< Line spacing (excluding font height) in units
    bbUINT font;            //!< Logical font number (local for each ptGC)
    bbUINT textfgcol;       //!< Initial colour number for text FG
    ptPEN  textbgpen;       //!< Initial pen for text BG
    ptPEN  bgpen;           //!< Pen for unused BG area

    #if ptTEXTBOX_HEAPH != 1
    const bbCHAR** pLines;  /**< Pointer to list of pointers to markup text lines.
                                 Each line must be in the format as described in #ptGC::MarkupText.
                                 This list is terminated with NULL.
                            */
    #else
    const bbHEAPH* pLines;  /**< Pointer to list of heap block handles to blocks containing markup text lines.
                                 Each line must be in the format as described in #ptGC::MarkupText.
                            */
    bbUINT linecount;       /**< Number of entries in \a pLines[] */
    #endif
} ptTextBox;

/** Lookup tables for ptGC::MarkupText(). */
struct ptMarkupInfo
{
    ptFont* mpFont[4];  //!< Font instances
    bbU16   mBGPen[8];  //!< Pens for text background
    bbU8    mFGCol[32]; //!< Logical colour numbers for text foreground
};

/** Interface to graphics context.

    A graphics context is a rectangular drawing area.
    Coordinate origin is always is in the top-left border.
    Coordinates are measured in \a units which are a multiple
    of actual pixel coordinates.

    <pre>
    +-------------->
    |(0,0)
    |
    v
    </pre>
*/
class ptGC
{
protected:
    ptPal* mpLogPal;        //!< Logical RGB palette

public:
    /** @name Properties
    @{*/

    /** Get width in units.
        @return With in units.
    */
    virtual bbUINT GetWidth() const = 0;

    /** Get height in units.
        @return Height in units.
    */
    virtual bbUINT GetHeight() const = 0;

    /** Get width in pixels.
        @return With in pixels.
    */
    inline bbUINT GetPixelWidth() const { return GetWidth() >> ptGCEIGHTX; }

    /** Get height in pixels.
        @return Height in pixels.
    */
    inline bbUINT GetPixelHeight() const { return GetHeight() >> ptGCEIGHTY; }

    /** Reset clip box to graphic context's full width and height. */
    void ResetClipBox();

    /** Set clip box.
        @param pRect Clip rectangle, left-top inclusive, right-bottom exclusive
    */
    inline void SetClipBox(ptRect* const pRect)
    {
        SetClipBox(pRect->left, pRect->top, pRect->right, pRect->bottom);
    }

    /** Set clip box.
        @param clipminx Minx (inclusive)
        @param clipminy Miny (inclusive)
        @param clipmaxx Maxx (exclusive)
        @param clipmaxy Maxy (exclusive)
    */
    virtual void SetClipBox(const int clipminx, const int clipminy, const int clipmaxx, const int clipmaxy) = 0;

    /** Get clip box.
        @param pRect Returns clipping box
    */
    virtual void GetClipBox(ptRect* const pRect) = 0;

    /** Set logical RGB palette.

        Function is virtual to allow ptGC implementations to update any colour lookup tables.
        Default implementation writes \a pLogRGB to \a mpLogRGB

        @param pLogRGB Palette to map logical colour numbers to RGB values.
                       The palette size is user-defined and depends on the number of used logical colour numbers.
                       The object remains under external control and must be available for ptGC's lifetime
        @return Previous logical RGB palette, does not neccessarily need to be reselected later
    */
    virtual ptPal* SetLogPal(ptPal* const pLogRGB);

    /** Map RGB colour to closest logical colour.
        @param rgb RGB colour, 32-bit value format: 0x00BBGGRR.
        @return Closest logical colour, index into table that was set with ptGC::SetLogPal()
    */
    inline bbUINT RGB2LogCol(bbU32 const rgb) const
    {
        return ptRGBPalMatch(mpLogPal->mpRGB, (bbUINT)mpLogPal->mColCount, rgb);
    }

    /*@}*/

    /** Clear the context with given colour.
        @param col Logical colour number
    */
    virtual void Clear(const bbUINT col) = 0;

    /** Set a pixel with given colour.
        @param x X-coordinate in units
        @param y Y-coordinate in units
        @param col Logical colour number
    */
    virtual void Point(const int x, const int y, const bbUINT col) = 0;

    /** Draw a horizontal line with given colour.
        The line will be drawn from the start position in positive x-direction.
        @param x X-coordinate of line start in units
        @param y Y-coordinate of line start in units
        @param width Width of line (inclusive) in units
        @param col Logical colour number
    */
    virtual void HLine(int x, int y, bbUINT width, bbUINT col) = 0;

    /** Draw a vertizontal line with given colour.
        The line will be drawn from the start position in positive y-direction.
        @param x X-coordinate of line in units
        @param y Y-coordinate of line start in units
        @param height Height of line (inclusive) in units
        @param col Logical colour number
    */
    virtual void VLine(int x, int y, bbUINT height, bbUINT col) = 0;

    /** Draw a rectangular box outline.
        @param x X-coordinate of top-left corner in units
        @param y Y-coordinate of top-left corner in units
        @param width Width in units (inclusive)
        @param height Height in units (inclusive)
        @param col Logical colour number
    */
    virtual void Box(const int x, const int y, const bbUINT width, const bbUINT height, const bbUINT col);

    /** Draw a rectangular filled box.
        @param x X-coordinate of top-left corner in units
        @param y Y-coordinate of top-left corner in units
        @param width Width in units (inclusive)
        @param height Height in units (inclusive)
        @param pen Pen to use
    */
    virtual void FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen) = 0;

    /** Draw a line with given colour.
        @param x1 X-coordinate of line start in units
        @param y1 Y-coordinate of line start in units
        @param x2 X-coordinate of line end in units
        @param y2 Y-coordinate of line end in units
        @param pen Pen to use
    */
    virtual void Line(int x1, int y1, int x2, int y2, const ptPEN pen) = 0;

    /** Draw a line string.
        @param pPoints Pointer to array of points
        @param pointcount Number of points
        @param pen Pen to use
    */
    void LineString(const ptCoord* const pPoints, const bbUINT pointcount, const ptPEN pen);

    /** Draw a linear ring (closed line string).
        @param pPoints Pointer to array of points
        @param pointcount Number of points
        @param pen Pen to use
    */
    void LinearRing(const ptCoord* const pPoly, const bbUINT pointcount, const ptPEN pen);

    /** Draw a filled konkav polygon.
        @param pPoints Pointer to array of points
        @param pointcount Number of points
        @param pen Pen to use
    */
    virtual void Polygon(const ptCoord* const pPoly, const bbUINT pointcount, const ptPEN pen) = 0;

    /** Draw a filled circle.
        @param x X-coordinate of center in units
        @param y Y-coordinate of center in units
        @param rad Radius in units
        @param pen Pen to use
    */
    virtual void FillCircle(int x, int y, bbUINT rad, const ptPEN pen) = 0;

    /** Draw character for given code point.
        @param x X-coordinate in units, designates left edge of textbox
        @param y Y-coordinate in units, designates top edge of textbox
        @param fgcol Text foreground logical colour number.
        @param bgpen Text background pen.
        @param font  Font
        @return Pixelwidth plotted (relative to \a x)
    */
    bbUINT Char(const int x, const int y, const bbCHARCP cp, bbUINT const fgcol, ptPEN const bgpen, const bbUINT font);

    /** Draw a line of escaped text.
        @param x X-coordinate in units, designates left edge of textbox
        @param y Y-coordinate in units, designates top edge of textbox
        @param pText   Buffer containing escaped string in character encoding #bbENC.
                       The buffer must be terminated as described below.
                       Escape character is 0:
                       <table>
                       <tr><td>0,0  </td><td>character 0</td></tr>
                       <tr><td>0,1  </td><td>line termination</td></tr>
                       <tr><td>0,2,c</td><td>set FG colour to \a c</td></tr>
                       <tr><td>0,3,p</td><td>set BG pen to \a p</td></tr>
                       <tr><td>0,4,f</td><td>set font to \a f. The new font must be of the
                                             same pixel height as \a font, behaviour is undefined
                                             otherwise.</td></tr>
                       <tr><td>0,5,c0,..,cN</td>
                                         <td>Some encodings dont allow to encode every codepoint.
                                             Using this escape sequence any codepoint can be addressed.
                                             The number of cN depends on the size of type bbCHARCP.
                                             - sizeof(bbCHARCP) == 1 -> c0
                                             - sizeof(bbCHARCP) == 2 -> c0,c1
                                             - sizeof(bbCHARCP) == 4 -> c0,c1,c2,c3
                                             c0,..,cN is specified in little endian order.
                                         </td></tr>
                       </table>
        @param fgcol Initial value for text foreground logical colour number.
        @param bgpen Initial value for text background pen. If the text background is changed in
                     the string by an escape code, only the ptPEN bitfield <i>Pen option 1</i> is
                     affected. Pen type and option 2 is preserved.
        @param font  Initial font
        @return Width plotted in units
    */
    virtual bbUINT Text(int x, int y, const bbCHAR* pText, bbUINT fgcol, ptPEN bgpen, bbUINT const font);

    /** Draw a line of markup text.
        @param x X-coordinate in units, designates left edge of textbox
        @param y Y-coordinate in units, designates top edge of textbox
        @param pText Buffer containing line of character code points, format is:
                     <pre>
                     Bit  0..21 - Character code point
                     Bit 22..23 - Font number 0-3
                     Bit 24..28 - FG colour number 0-31
                     Bit 29..31 - BG pen number 0-7
                     </pre>
                     Font, FG, and BG are indexes into corresponding ptMarkupInfo members.
                     The line must be terminated with 0xFFFFFFFFUL.
                     Each words stands for a half-width character. For wide characters, 2 bbU32 words
                     are used, where the 1st word holds all information and the 2nd word's low 22 bits must be 0x3FFFFEUL.
        @param pInfo Lookup tables for font and colour numbers
        @return Width plotted in units
    */
    virtual bbUINT MarkupText(int /*x*/, int /*y*/, const bbU32* /*pText*/, const ptMarkupInfo* const /*pInfo*/, bbUINT const /*linespacing*/) { return 0; } //xxx

    #define ptGCMT_MASK ((1<<22)-1)
    #define ptGCMT_FONTPOS 22
    #define ptGCMT_FONTMASK 3
    #define ptGCMT_FGPOS 24
    #define ptGCMT_FGMASK 31
    #define ptGCMT_BGPOS 29

    /** Draw a text box with multiple lines of markup text.
        The plotarea is drawn to the screen according to the scroll position
        and textbox width. Additional clipping is done for the currently set cliparea.
        @param pTextbox Structure describing position and contents of textbox
    */
    void MarkupTextBox(ptTextBox* const pTextbox);

    /** Draw a sprite.
        @param x X-coordinate in units, designates left edge of sprite
        @param y Y-coordinate in units, designates top edge of sprite
        @param pSprite Pointer to sprite
    */
    virtual void Sprite(int x, int y, const ptSprite* const pSprite) = 0;
};

#endif /* ptGC_H_ */
