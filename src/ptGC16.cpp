#include "ptGC16.h"

#if ptUSE_GC16 == 1

#include <babel/mem.h>
#include <babel/fixmath.h>
#include <babel/str.h>
#include "ptSprite.h"

ptGC16::ptGC16()
{
    mpColLU = NULL;
}

ptGC16::~ptGC16()
{
    bbMemFreeNull( (void**)&mpColLU);
}

void ptGC16::Clear(const bbUINT col)
{
}

void ptGC16::Point(const int x, const int y, const bbUINT col)
{
    if ((x >= mUnitClipMinX) && (x < mUnitClipMaxX) && (y >= mUnitClipMinY) && (y < mUnitClipMaxY))
    {
        *((bbU16*)mpBmp + (y>>ptGCEIGHTY)*mPixPitch + (x>>ptGCEIGHTX)) = mpColLU[col];
    }
}

void ptGC16::HLine(int x, int y, bbUINT width, bbUINT col)
{
}

void ptGC16::VLine(int x, int y, bbUINT height, bbUINT col)
{
}

void ptGC16::FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen)
{
}

void ptGC16::Line(int x1, int y1, int x2, int y2, const ptPEN pen)
{

}

void ptGC16::Polygon(const ptCoord* const pPoints, const bbUINT pointcount, const ptPEN pen)
{
}

void ptGC16::FillCircle(int x, int y, bbUINT rad, const ptPEN pen)
{
}

bbUINT ptGC16::Text(int x, int y, const bbCHAR* pMarkup, bbUINT fgcol, ptPEN bgpen, bbUINT const font)
{
    return 0;
}

void ptGC16::Sprite(int x, int y, const ptSprite* const pSprite)
{
    bbU8* pBmp = (bbU8*)mpBmp;
    bbU8* pSpr = (bbU8*) &pSprite->data[0];
    bbUINT bmppitch;
    bbUINT sprpitch;

    switch (pSprite->colfmt)
    {
    case ptCOLFMT_RGB565:
        sprpitch = ((pSprite->width << 1) + 3) &~ 3;
        bmppitch = mPixPitch<<1;
        bbUINT lpy = pSprite->height;
        while (lpy)
        {
            bbMemMove( pBmp, pSpr, sprpitch);
            pSpr+=sprpitch;
            pBmp+=bmppitch;
            --lpy;
        }
        break;
    }
}

#endif /* ptUSE_GC16 == 1 */
