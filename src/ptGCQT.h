#ifndef ptGCQT_H_
#define ptGCQT_H_

#include "ptGC.h"

#if ptUSE_GCQT == 1

class QPainter;
class QImage;

class ptGCQT : public ptGC
{
private:
    QPainter*   mpPainter;
    bbUINT      mWidth;         //!< Width in units
    bbUINT      mHeight;        //!< Height in units
    QImage*     mpLineCache;

    bbERR EnsureLineCache(bbUINT width, bbUINT height);

public:
    ptGCQT(QPainter* pPainter = NULL);
    ~ptGCQT();

    void AttachPainter(QPainter* pPainter);

    // ptGC interface implementation
    virtual bbUINT GetWidth() const;
    virtual bbUINT GetHeight() const;
    virtual void SetClipBox(const int clipminx, const int clipminy, const int clipmaxx, const int clipmaxy);
    virtual void GetClipBox(ptRect* const pRect);
    virtual void Clear(const bbUINT col);
    virtual void Point(const int x, const int y, const bbUINT col);
    virtual void HLine(int x, int y, bbUINT width, bbUINT col);
    virtual void VLine(int x, int y, bbUINT height, bbUINT col);
    virtual void Box(const int x, const int y, const bbUINT width, const bbUINT height, const bbUINT col);
    virtual void FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen);
    virtual void Line(int x1, int y1, int x2, int y2, const ptPEN pen);
    virtual void Polygon(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen);
    virtual void FillCircle(int x, int y, bbUINT rad, const ptPEN pen);
    virtual bbUINT Text(int x, int y, const bbCHAR* pMarkup, bbUINT fgcol, ptPEN bgpen, bbUINT const font);
    virtual bbUINT MarkupText(int x, int y, const bbU32* pText, const ptMarkupInfo* const pInfo, bbUINT const linespacing);
    virtual void Sprite(int x, int y, const ptSprite* const pSprite);
};

#endif
#endif
