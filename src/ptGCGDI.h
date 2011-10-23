#ifndef ptGCGDI_H_
#define ptGCGDI_H_

#include "ptGC.h"

#if ptUSE_GCGDI == 1

class ptGCGDI : public ptGC
{
private:
    HDC         mhDC;           //!< Device context
    HBITMAP     mhMask;         //!< Monochrome temp bitmap, used to blit ptFont's
    BITMAPINFO* mpBMI;          //!< BITMAP info struct for Sprite painting
    bbU8*       mpMask;         //!< Pixel data for mhMask
    bbU8*       mpLineCache;    //!< Pixel line memory colour format conversion in Sprite
    bbU32       mLineCacheSize; //!< Size of mpLineCache buffer in bytes
    bbUINT      mWidth;         //!< Width in units
    bbUINT      mHeight;        //!< Height in units
    bbU32       mCachedPal;     //!< Hash of cached palette
    bbUINT      mMaskWidth;     //!< Current width of mhMask
    bbUINT      mMaskHeight;    //!< Current height of mhMask

    HPEN        mhDotPen;

private:
    bbERR EnsureLineCache(bbUINT const linesize);
    void CachePal(ptPal* const pPal, bbUINT size);
    bbERR EnsureMaskBmp(bbUINT const width, bbUINT const height);

public:
    ptGCGDI();
    ~ptGCGDI();

    void AttachDC(HDC const hDC);
    inline void AttachDC0(HDC const hDC) { mhDC = hDC; }
    inline HDC DetachDC() { HDC const hDC = mhDC; mhDC = NULL; return hDC; }
    inline void SetWidth(const bbUINT width) { mWidth = width << ptGCEIGHTX; }
    inline void SetHeight(const bbUINT height) { mHeight = height << ptGCEIGHTY; }

    inline void SetClipBox(ptRect* const pRect)
    {
        SetClipBox(pRect->left, pRect->top, pRect->right, pRect->bottom);
    }

    bbERR Init();
    void Destroy();
    inline int IsInitialized() const { return mpBMI != NULL; }

    // ptGC interface implementation
    virtual bbUINT GetWidth() const;
    virtual bbUINT GetHeight() const;
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
    virtual bbUINT MarkupText(int x, int y, const bbU32* pText, const ptMarkupInfo* const pInfo, bbUINT const linespacing);
    virtual void Sprite(int x, int y, const ptSprite* const pSprite);
};

#endif /* ptUSE_GCGDI */

#endif /* ptGCGDI_H_ */
