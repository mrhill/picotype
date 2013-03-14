#ifndef ptGC16_H_
#define ptGC16_H_

/** @file
    16bpp true colour custom graphics context with 32k colours.
*/

#include "ptGC.h"
#include "ptdefs.h"

#if ptUSE_GC16 == 1

#if ptUSE_16BPP != 1
#error ptUSE_16BPP must be defined to 1 to compile ptGC16
#endif

/** Implementation of graphics context, 16 bpp true colour. */
class ptGC16 : public ptGC
{
private:
    bbU16*       mpBmp;     //!< Pointer to start of bitmap (usage specific to implementation)
    bbUINT       mPixPitch; //!< Pitch in pixels
    const bbU16* mpColLU;   //!< Lookup-table for logical -> physical colours, managed internally

    bbUINT       mUnitWidth;    //!< Width in units
    bbUINT       mUnitHeight;   //!< Height in units

    int          mUnitClipMinX;
    int          mUnitClipMaxX;
    int          mUnitClipMinY;
    int          mUnitClipMaxY;

public:
    ptGC16();
    ~ptGC16();

    /** Get width in units.
        @return With in units.
    */
    virtual bbUINT GetWidth() const { return mUnitWidth; }

    /** Get height in units.
        @return Height in units.
    */
    virtual bbUINT GetHeight() const { return mUnitHeight; }

    /** Get bitmap pointer.
        @return Pointer to pixel data.
    */
    inline bbU16* GetBmp() const { return mpBmp; }

    /** Set bitmap pointer.
        @param pBmp Pointer to pixel data.
    */
    inline void SetBmp(bbU16* const pBmp) { mpBmp = pBmp; }

    /** Get pitch in pixels.
        @return Pitch in pixels
    */
    inline bbUINT GetPitch() const { return mPixPitch; }

    virtual void Clear(const bbUINT col);
    virtual void Point(const int x, const int y, const bbUINT col);
    virtual void HLine(int x, int y, bbUINT width, bbUINT col);
    virtual void VLine(int x, int y, bbUINT height, bbUINT col);
    virtual void FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen);
    virtual void Line(int x1, int y1, int x2, int y2, const ptPEN pen);
    virtual void Polygon(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen);
    virtual void FillCircle(int x, int y, bbUINT rad, const ptPEN pen);
    virtual void Sprite(int x, int y, const ptSprite* const pSprite);
};

#endif /* ptUSE_GC16 == 1 */

#endif /* ptGC16_H_ */
