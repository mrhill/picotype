#ifndef ptGC8_H_
#define ptGC8_H_

/** @file
    8bpp palette-indexed custom graphics context.
*/

#include "ptGC.h"
#include "ptdefs.h"

#if ptUSE_GC8 == 1

/** Implementation of graphics context, 8bpp with indexed palette. */
class ptGC8 : public ptGC
{
private:
    bbU8*        mpBmp;             //!< Pointer to start of bitmap (usage specific to implementation)
    bbUINT       mPixPitch;         //!< Pitch in pixels
    const bbU8*  mpColLU;           //!< Lookup-table for logical -> physical colour numbers, managed internally
    const bbU8*  mpAALU;            //!< Lookup-table for Anti-aliasing, managed internally
    const bbU32* mpRGB;             //!< Currently set RGB palette (256 32-bit words), managed externally

    bbUINT       mUnitWidth;        //!< Width in units
    bbUINT       mUnitHeight;       //!< Height in units

    int          mUnitClipMinX;
    int          mUnitClipMaxX;
    int          mUnitClipMinY;
    int          mUnitClipMaxY;

public:
    ptGC8();
    ~ptGC8();

    bbERR Init();
    void Destroy();

    /** Get bitmap pointer.
        @return Pointer to pixel data. 
    */
    inline bbU8* GetBmp() const { return mpBmp; }

    /** Set bitmap pointer.
        @param pBmp Pointer to pixel data. 
    */
    inline void SetBmp(bbU8* const pBmp) { mpBmp = pBmp; }

    /** Set pitch in pixels.
        @param pitch Pitch in pixels
    */
    inline void SetPitch(const bbUINT pitch) { mPixPitch = pitch; }

    /** Get pitch in pixels.
        @return Pitch in pixels
    */
    inline bbUINT GetPitch() const { return mPixPitch; }

    /** Set new width in pixels.
        @param Width in pixels
    */
    inline void SetPixelWidth(const bbUINT width) { mUnitWidth = width << ptGCEIGHTX; }

    /** Set new height in pixels.
        @param Height in pixels
    */
    inline void SetPixelHeight(const bbUINT height) { mUnitHeight = height << ptGCEIGHTY; }

    /** Get width in units.
        @return With in units.
    */
    virtual bbUINT GetWidth() const { return mUnitWidth; }

    /** Get height in units.
        @return Height in units.
    */
    virtual bbUINT GetHeight() const { return mUnitHeight; }

    /** Set 256 entry physical RGB palette.

        This palette maps 8 bit physical colour number to RGB colours.

        Setting a palette will cause the internal anti-alias lookup table
        to be recalculated. Therefore this call is slow.

        Sets #bbgErr on failure.

        @param pPal    Physical palette
                       This block remains under external control, and must be available
                       for the lifetime of ptGC8.
        @param pAAcols Pointer to #ptGCAASIZE entries table specifying which
                       physical colours to include in the anti-alias lookup table.
                       Set to NULL to use the first 16 colours.
        @return bbEOK on success, or error code on failure.
    */
    bbERR SetPal(ptPal* const pPal, const bbU8* const pAAcols = NULL);

    /** Get pointer to currently set RGB palette.
        @return Pointer to 256 entries RGB table, value format: 0x00BBGGRR.
    */
    inline const bbU32* GetPal() const { return mpRGB; }

    virtual void SetClipBox(const int clipminx, const int clipminy, const int clipmaxx, const int clipmaxy);
    virtual void GetClipBox(ptRect* const pRect);
    virtual void Clear(const bbUINT col);
    virtual void Point(const int x, const int y, const bbUINT col);
    virtual void HLine(int x, int y, bbUINT width, bbUINT col);
    virtual void VLine(int x, int y, bbUINT height, bbUINT col);
    virtual void FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen);
    virtual void Line(int x1, int y1, int x2, int y2, const ptPEN pen);
    virtual void Polygon(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen);
    virtual void FillCircle(int x, int y, bbUINT rad, const ptPEN pen);
    virtual bbUINT Text(int x, int y, const bbCHAR* pMarkup, bbUINT fgcol, ptPEN bgpen, bbUINT const font);
    virtual void Sprite(int x, int y, const ptSprite* const pSprite);

    #ifdef bbDEBUG
    void DebugAA();
    #endif
};

#endif /* ptUSE_GC8 == 1 */

#endif /* ptGC_H_ */
