#include "ptGC8.h"
#include "ptFontMan.h"
#include "ptPalMan.h"

#if ptUSE_GC8 == 1

#include <babel/mem.h>
#include <babel/fixmath.h>
#include <babel/str.h>
#include "ptSprite.h"

#define ptGCSCRATCHSIZE 0x1000U
static bbU8 gScratch[ptGCSCRATCHSIZE]; //!< Scratch space shared by all instances

ptGC8::ptGC8()
{
    mpBmp       = NULL;
    mpColLU     = NULL;
    mpAALU      = NULL;
    mpRGB       = NULL;
}

ptGC8::~ptGC8()
{
    Destroy();
}

bbERR ptGC8::Init()
{
    mPixPitch   = 0;//pitch;
    mUnitWidth  = 0;//width << ptGCEIGHTX;
    mUnitHeight = 0;//height << ptGCEIGHTY;

    ResetClipBox();

    return bbEOK;
}

void ptGC8::Destroy()
{
    bbMemFreeNull( (void**) &mpAALU);
}

void ptGC8::SetClipBox(const int clipminx, const int clipminy, const int clipmaxx, const int clipmaxy)
{
    mUnitClipMinX = clipminx;
    mUnitClipMaxX = clipmaxx;
    mUnitClipMinY = clipminy;
    mUnitClipMaxY = clipmaxy;
}

void ptGC8::GetClipBox(ptRect* const pRect)
{
    pRect->left   = mUnitClipMinX;
    pRect->top    = mUnitClipMinY;
    pRect->right  = mUnitClipMaxX;
    pRect->bottom = mUnitClipMaxY;
}

bbERR ptGC8::SetPal(ptPal* const pPalRGB, const bbU8* const pAAcols)
{
    bbUINT i, j, k;

    // Count number of logical colours

    ptPal* const pLogPal = mpLogPal;
    bbASSERT(pLogPal);

    // Get mem for lookup tables

    bbU8* pAALU = (bbU8*) mpAALU;
    const bbERR err = bbMemRealloc( 256 + ptGCAASIZE*ptGCAASIZE*(1<<ptGCAAPREC) + pLogPal->mColCount, (void**) &pAALU);
    if (err != bbEOK) return err;

    // Save pointer to physical palette

    const bbU32* pRGB = pPalRGB->mpRGB;
    bbASSERT(pRGB);
    mpRGB = pRGB;

    // calculate lookup logical colour number -> physical colour number

    bbU8* const pColLU = pAALU + (256+(1<<ptGCAAPREC)*ptGCAASIZE*ptGCAASIZE);
    mpColLU = pColLU;

    for (i=0; i<pLogPal->mColCount; ++i)
    {
        pColLU[i] = ptRGBPalMatch(pRGB, 256, pLogPal->mpRGB[i]);
    }

    // calculate lookup physical colour number -> AA lookup table index

    for (i=0; i<256; ++i)
    {
        bbU32  bestdist = ptRGBDist(pRGB[i], pRGB[ pAAcols ? pAAcols[0] : 0 ]);
        bbUINT bestidx = 0;

        for (j=1; j<ptGCAASIZE; ++j)
        {
            const bbU32 dist = ptRGBDist(pRGB[i], pRGB[ pAAcols ? pAAcols[j] : j ]);
            if (dist < bestdist)
            {
                bestdist = dist;
                bestidx = j;
            }
        }

        pAALU[i] = (bbU8) bestidx;
    }

    // calculate AA lookup tables

    bbU8* const pAA = pAALU + 256;

    #if (1<<ptGCAAPREC)==8
    static const bbU8 aascales[1<<ptGCAAPREC] = { 0x00,0x12,0x24,0x36,0x49,0x5B,0x6D,0x80 };
    #else
    bbU8 aascales[1<<ptGCAAPREC];
    for (i=0; i<(1<<ptGCAAPREC); ++i) aascales[i] = (bbU8) ((i<<(ptGCAAPREC+4)) / ((1<<ptGCAAPREC)-1));
    #endif

    for (i=0; i<ptGCAASIZE; ++i)
    {
        const bbU32 rgb_from = pRGB[ pAAcols ? pAAcols[i] : i ];

        for (j=0; j<ptGCAASIZE; ++j)
        {
            const bbU32 rgb_to = pRGB[ pAAcols ? pAAcols[j] : j ];

            for (k=0; k<(1<<ptGCAAPREC); ++k)
            {
                pAA[(i<<(ptGCAASIZE_LOG2+ptGCAAPREC))+(j<<ptGCAAPREC)+k] = ptRGBPalMatch(pRGB, 256, ptGetWeightedRGB(rgb_from, rgb_to, aascales[k] << (16-(4+ptGCAAPREC))));
            }
        }
    }

    mpAALU = pAALU;
    return bbEOK;
}

#ifdef bbDEBUG
void ptGC8::DebugAA()
{
    int x, y, i;
    const bbU8* const pAALU = mpAALU;
    const bbU8* const pAA = mpAALU + 256;

    for (y=0; y<16; ++y)
        for (x=0; x<16; ++x)
            FillBox( x*(4<<ptGCEIGHTX), y*(4<<ptGCEIGHTY), 4<<ptGCEIGHTX, 4<<ptGCEIGHTY, x|(y<<4));

    for (y=0; y<16; ++y)
        for (x=0; x<16; ++x)
            FillBox( x*(4<<ptGCEIGHTX), y*(4<<ptGCEIGHTY)+(80<<ptGCEIGHTY), 4<<ptGCEIGHTX, 4<<ptGCEIGHTY, pAA[(bbUINT)pAALU[x|(y<<4)]<<(ptGCAASIZE_LOG2+ptGCAAPREC)]);

    for (y=0; y<ptGCAASIZE; ++y)
        for (x=0; x<ptGCAASIZE; ++x)
            for (i=0; i<(1<<ptGCAAPREC); ++i)
                FillBox(((y<<ptGCAAPREC) + i)<<ptGCEIGHTX, x*(4<<ptGCEIGHTY)+(160<<ptGCEIGHTY), 4<<ptGCEIGHTX, 4<<ptGCEIGHTY, pAA[i + (x<<ptGCAAPREC) + (y<<(ptGCAASIZE_LOG2+ptGCAAPREC))] );
}
#endif

void ptGC8::Clear(const bbUINT col)
{
    const bbUINT width   = (GetPixelWidth()+3)&~3; // clears more than allowed if no multiple of 4
    const bbUINT pitch   = GetPitch();
    bbU32*       pScr    = (bbU32*) GetBmp();
    bbU32* const pScrEnd = (bbU32*) ((bbU8*)pScr + GetPixelHeight() * pitch);

    register bbU32 c = mpColLU[col];
    c |= c<<8;
    c |= c<<16;

    while (pScr < pScrEnd)
    {
        register bbU32* pTmp = pScr;
        bbU32* const pLineEnd = pTmp + (width>>2);

        do { *(pTmp++) = c; } while (pTmp<pLineEnd); // clears at least 1 column, even if width is 0

        pScr += pitch >> 2;
    }
}

void ptGC8::Point(const int x, const int y, const bbUINT col)
{
    if ((x >= mUnitClipMinX) && (x < mUnitClipMaxX) && (y >= mUnitClipMinY) && (y < mUnitClipMaxY))
    {
        *(mpBmp + (y>>ptGCEIGHTY)*mPixPitch + (x>>ptGCEIGHTX)) = mpColLU[col];
    }
}

void ptGC8::HLine(int x, int y, bbUINT width, bbUINT col)
{
    if ((y < mUnitClipMinY)||(y >= mUnitClipMaxY))  return;

    int x2 = x + width;
    if (x  < mUnitClipMinX)  x  = mUnitClipMinX;
    if (x2 > mUnitClipMaxX)  x2 = mUnitClipMaxX;
    if (x >= x2)  return;

    col = mpColLU[col];

    x >>= ptGCEIGHTX;
    x2 >>= ptGCEIGHTX;
    bbMemSet( GetBmp() + (y>>ptGCEIGHTY) * GetPitch() + x, col, x2-x);
}

void ptGC8::VLine(int x, int y, bbUINT height, bbUINT col)
{
    if ((x < mUnitClipMinX)||(x >= mUnitClipMaxX))  return;

    int y2 = y + height;
    if (y  < mUnitClipMinY)  y  = mUnitClipMinY;
    if (y2 > mUnitClipMaxY)  y2 = mUnitClipMaxY;
    if (y >= y2)  return;

    col = mpColLU[col];

    const bbUINT pitch = GetPitch();
    y >>= ptGCEIGHTY;
    y2 >>= ptGCEIGHTY;
    bbU8* pTmp = GetBmp() + y * pitch + (x>>ptGCEIGHTX);
    y2 -= y;
    do
    {
        *pTmp = col;
        pTmp += pitch;

    } while (--y2>0);
}

void ptGC8::FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen)
{
    int x2 = x + width;
    if (x  < mUnitClipMinX)  x  = mUnitClipMinX;
    if (x2 > mUnitClipMaxX)  x2 = mUnitClipMaxX;
    if (x >= x2)  return;

    int y2 = y + height;
    if (y  < mUnitClipMinY)  y  = mUnitClipMinY;
    if (y2 > mUnitClipMaxY)  y2 = mUnitClipMaxY;
    if (y >= y2)  return;

    const bbUINT pitch = GetPitch();
    x >>= ptGCEIGHTX;
    bbU8* pTmp = GetBmp() + (y>>ptGCEIGHTY)*pitch + x;
    x = (x2>>ptGCEIGHTX) - x;

    bbASSERT(y < y2);

    if ((pen & ptPENMASK_TYPE) != ptPEN_TRANS)
    {
        pen = mpColLU[ pen &~ ptPENMASK ];
        do
        {
            bbMemSet( pTmp, pen, x);
            pTmp += pitch;
            y += (1<<ptGCEIGHTY);

        } while (y < y2);
    }
    else
    {
        const bbU8* const pAALU = mpAALU;
        const bbU8* const pAA = pAALU + 256 + (pen>>ptPENBITPOS_OPT2) + ((bbUINT)pAALU[mpColLU[pen &~ ptPENMASK]]<<(ptGCAASIZE_LOG2+ptGCAAPREC));
        do
        {
            const bbU8* const pEnd = pTmp + x;
            while (pTmp < pEnd)
            {
                *pTmp = pAA[(bbUINT)pAALU[*pTmp]<<ptGCAAPREC];
                pTmp++;
            }
            pTmp += pitch-x;
            y += (1<<ptGCEIGHTY);

        } while (y < y2);
    }
}

void ptGC8::Line(int x1, int y1, int x2, int y2, const ptPEN pen)
{
    int dx, dy;

    // distinct between horizontal and vertical loop

    if ((dx = x2-x1)<0) dx=-dx;
    if ((dy = y2-y1)<0) dy=-dy;

    if (dx >= dy)
    {
        if (x2<x1) // sort coords
        {
            const int tmpx = x1; x1 = x2; x2 = tmpx;
            const int tmpy = y1; y1 = y2; y2 = tmpy;
        }

        if (dx >= bbDIVTABSIZE) // prevent divtab overflow
        {
            Line(x1, y1, (x1+x2)>>1, (y1+y2)>>1, pen);
            Line((x1+x2)>>1, (y1+y2)>>1, x2, y2, pen);
            return;
        }

        if ((x2 < mUnitClipMinX) || (x1 >= mUnitClipMaxX)) return;

        if ((dy = y2-y1) >= 0)
        {
            if ((y1 >= mUnitClipMaxY) || (y2 < mUnitClipMinY)) return;
        }
        else
        {
            if ((y2 >= mUnitClipMaxY) || (y1 < mUnitClipMinY)) return;
        }

        const long step = (long)bbgpDivTab[dx] * dy; // dx is < bbDIVTABSIZE

        if (x1 < mUnitClipMinX)
        {
            y1 += (int)(((long)(mUnitClipMinX - x1) * step) >> bbDIVTABPREC);
            x1 = mUnitClipMinX;
        }

        // Complicated clipping, we might leave this out.
        // If MULs are expensive this additional clipping might not pay off.
        if (dy >= 0)
        {
            if (y1 < mUnitClipMinY)
            {
                bbUINT d = dy;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                x1 += (int)(((long)bbgpDivTab[d] * dx * (mUnitClipMinY-y1)) >> s);
                if (x1 >= mUnitClipMaxX) return;
                y1 = mUnitClipMinY;
            }

            if (y2 >= mUnitClipMaxY)
            {
                bbUINT d = dy;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                x2 += (int)(((long)bbgpDivTab[d] * dx * (mUnitClipMaxY-y2)) >> s);
                if (x2 < mUnitClipMinX) return;
                // y2 not needed
            }
        }
        else
        {
            if (y1 >= mUnitClipMaxY)
            {
                bbUINT d = -dy;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                x1 += (int)(((long)bbgpDivTab[d] * dx * (y1-mUnitClipMaxY)) >> s);
                if (x1 >= mUnitClipMaxX) return;
                y1 = mUnitClipMaxY - 1;
            }

            if (y2 < mUnitClipMinY)
            {
                bbUINT d = -dy;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                x2 += (int)(((long)bbgpDivTab[d] * dx * (y2-mUnitClipMinY)) >> s);
                if (x2 < mUnitClipMinX) return;
                // y2 not needed
            }
        }

        if (x2 >= mUnitClipMaxX)  x2 = mUnitClipMaxX;
        dx = (x2 - x1) >> ptGCEIGHTX;

        const int pitch = (step>=0) ? (int)GetPitch() : -(int)GetPitch();

        bbU8* const pScrStart = GetBmp() + (long)GetPitch()*(mUnitClipMinY>>ptGCEIGHTY);
        long offs = (long)GetPitch()*((y1-mUnitClipMinY)>>ptGCEIGHTY) + (x1>>ptGCEIGHTX);

        long y = (long)y1 << bbDIVTABPREC;
        long yo = (long)y1 >> ptGCEIGHTY;

        if ((pen & ptPENMASK_TYPE) != ptPEN_AA)
        {
            const bbU32 winsize = (long)GetPitch()*((mUnitClipMaxY-mUnitClipMinY)>>ptGCEIGHTY);
            const bbUINT col = mpColLU[ pen &~ ptPENMASK ];
            do
            {
                if ((bbU32)offs < winsize) *(pScrStart + offs) = col;
                offs++;
                y += step << ptGCEIGHTY;
                if ( (y>>(bbDIVTABPREC+ptGCEIGHTY)) != yo)
                {
                    yo = (y>>(bbDIVTABPREC+ptGCEIGHTY));
                    offs += pitch;
                }

            } while (--dx>0);
        }
        else
        {
            const bbU32 winsize = (long)GetPitch()*((mUnitClipMaxY-mUnitClipMinY)>>ptGCEIGHTY)-GetPitch();
            bbU8* const pScrStart2 = pScrStart + GetPitch();
            const bbU8* const pAALU = mpAALU;
            const bbU8* const pAA = pAALU + 256 + ((bbUINT)pAALU[mpColLU[pen &~ ptPENMASK]]<<(ptGCAASIZE_LOG2+ptGCAAPREC));
            do
            {
                if ((bbU32)offs < winsize)
                {
                    const bbUINT a = ((y>>(bbDIVTABPREC+ptGCEIGHTY-ptGCAAPREC)) & ((1<<ptGCAAPREC)-1));
                    *(pScrStart + offs)  = pAA[ ((bbUINT)pAALU[*(pScrStart  + offs)]<<ptGCAAPREC) + a];
                    *(pScrStart2 + offs) = pAA[ ((bbUINT)pAALU[*(pScrStart2 + offs)]<<ptGCAAPREC) + (a ^ ((1<<ptGCAAPREC)-1)) ];
                }
                offs++;
                y += step << ptGCEIGHTY;
                if ( (y>>(bbDIVTABPREC+ptGCEIGHTY)) != yo)
                {
                    yo = (y>>(bbDIVTABPREC+ptGCEIGHTY));
                    offs += pitch;
                }

            } while (--dx>0);
        }
    }
    else
    {
        if (y2<y1) // sort coords
        {
            const int tmpx = x1; x1 = x2; x2 = tmpx;
            const int tmpy = y1; y1 = y2; y2 = tmpy;
        }

        if (dy >= bbDIVTABSIZE) // prevent divtab overflow
        {
            Line(x1, y1, (x1+x2)>>1, (y1+y2)>>1, pen);
            Line((x1+x2)>>1, (y1+y2)>>1, x2, y2, pen);
            return;
        }

        if ((y2 < mUnitClipMinY) || (y1 >= mUnitClipMaxY)) return;

        if ((dx = x2-x1) >= 0)
        {
            if ((x1 >= mUnitClipMaxX) || (x2 < mUnitClipMinX)) return;
        }
        else
        {
            if ((x2 >= mUnitClipMaxX) || (x1 < mUnitClipMinX)) return;
        }

        const long step = (long)bbgpDivTab[dy] * dx; // dy is < bbDIVTABSIZE

        if (y1 < mUnitClipMinY)
        {
            x1 += (int)(((long)(mUnitClipMinY - y1) * step) >> bbDIVTABPREC);
            y1 = mUnitClipMinY;
        }

        // Complicated clipping.
        if (dx >= 0)
        {
            if (x1 < mUnitClipMinX)
            {
                bbUINT d = dx;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                y1 += (int)(((long)bbgpDivTab[d] * dy * (mUnitClipMinX-x1)) >> s);
                if (y1 >= mUnitClipMaxY) return;
                x1 = mUnitClipMinX;
            }

            if (x2 >= mUnitClipMaxX)
            {
                bbUINT d = dx;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                y2 += (int)(((long)bbgpDivTab[d] * dy * (mUnitClipMaxX-x2)) >> s);
                if (y2 < mUnitClipMinY) return;
                // x2 not needed
            }
        }
        else
        {
            if (x1 >= mUnitClipMaxX)
            {
                bbUINT d = -dx;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                y1 += (int)(((long)bbgpDivTab[d] * dy * (x1-mUnitClipMaxX)) >> s);
                if (y1 >= mUnitClipMaxY) return;
                x1 = mUnitClipMaxX - 1;
            }

            if (x2 < mUnitClipMinX)
            {
                bbUINT d = -dx;
                int s = bbDIVTABPREC;
                while (d >= bbDIVTABSIZE) d>>=1, s++;

                y2 += (int)(((long)bbgpDivTab[d] * dy * (x2-mUnitClipMinX)) >> s);
                if (y2 < mUnitClipMinY) return;
                // x2 not needed
            }
        }

        if (y2 >= mUnitClipMaxY)  y2 = mUnitClipMaxY;
        dy = (y2 - y1) >> ptGCEIGHTY;


        const bbUINT pitch = GetPitch();
        const int dir = step<0 ? -1 : 1;

        bbU8* const pScrStart = GetBmp() + (long)pitch*(mUnitClipMinY>>ptGCEIGHTY);
        const bbU32 winsize = (long)pitch*((mUnitClipMaxY-mUnitClipMinY)>>ptGCEIGHTY);
        long offs = (long)pitch*((y1-mUnitClipMinY)>>ptGCEIGHTY) + (x1>>ptGCEIGHTX);

        long x = (long)x1 << bbDIVTABPREC;
        long xo = (long)x1 >> ptGCEIGHTX;
        if ((pen & ptPENMASK_TYPE) != ptPEN_AA)
        {
            const bbUINT col = mpColLU[ pen &~ ptPENMASK ];
            do
            {
                if ((bbU32)offs < winsize) *(pScrStart + offs) = col;
                offs+=pitch;
                x += step << ptGCEIGHTX;
                if ( (x>>(bbDIVTABPREC+ptGCEIGHTX)) != xo)
                {
                    xo = (x>>(bbDIVTABPREC+ptGCEIGHTX));
                    offs += dir;
                }

            } while (--dy>0);
        }
        else
        {
            const bbU8* const pAALU = mpAALU;
            const bbU8* const pAA = pAALU + 256 + ((bbUINT)pAALU[mpColLU[pen &~ ptPENMASK]]<<(ptGCAASIZE_LOG2+ptGCAAPREC));
            do
            {
                if ((bbU32)offs < winsize)
                {
                    const bbUINT a = ((x>>(bbDIVTABPREC+ptGCEIGHTX-ptGCAAPREC)) & ((1<<ptGCAAPREC)-1));
                    *(pScrStart + offs) = pAA[ ((bbUINT)pAALU[*(pScrStart + offs)]<<ptGCAAPREC) + a];
                    *(pScrStart + offs + 1) = pAA[ ((bbUINT)pAALU[*(pScrStart + offs + 1)]<<ptGCAAPREC) + (a ^ ((1<<ptGCAAPREC)-1)) ];
                }
                offs+=pitch;
                x += step << ptGCEIGHTX;
                if ( (x>>(bbDIVTABPREC+ptGCEIGHTX)) != xo)
                {
                    xo = (x>>(bbDIVTABPREC+ptGCEIGHTX));
                    offs += dir;
                }

            } while (--dy>0);
        }
    }
}

typedef struct
{
    short y;
    short ye;
    long x;
    long sx;

} ptGCEDGE;

void ptGC8::Polygon(const ptCoord* const pPoints, const bbUINT pointcount, const ptPEN pen)
{
    // Scratch memory usage: 4096 bytes
    // bbU16    [256] bintree
    // bbU8     [256] bintree iteration stack
    // bbU8     [256] direction array / active edge index array
    // ptGCEDGE [256] bytes edges

    if (pointcount < 3) return;

    // y-enum loop. build a bintree for all y-coords

    bbU16* const pSort = (bbU16*) gScratch; // 512 bytes
    bbU8* const pAE = (bbU8*)pSort + 768;
    #define pFrom pAE

    // - find last y position distinct from first point (start direction)

    int y = pPoints[0].y >> ptGCEIGHTY;
    bbUINT di = pointcount;
    do
    {
        if (--di == 0) return; // the poly is only 1 scanline
    } while (y == (pPoints[di].y >> ptGCEIGHTY));

    *pFrom = (bbU8) di;
    *pSort = 0U;

    bbUINT i=1;
    do
    {
        const int yn = pPoints[i].y;

        if (y != (yn >> ptGCEIGHTY))
        {
            di = i-1;
            y = yn;
        }
        pFrom[i] = (bbU8) di;

        bbUINT jo;
        bbUINT j=0;
        do
        {
            jo=j;
            if (yn < pPoints[j].y) j=pSort[j]&0xFFU; else j=pSort[j]>>8;
        } while (j);

        pSort[jo] |= (yn < pPoints[jo].y) ? i : (i<<8);
        pSort[i] = 0U;

    } while (++i < pointcount);

    // y-sort loop. transform y-coord bintree into array of visible vertical edges,
    // sorted by start y-coords. Straight-horizontal edges and edges completely
    // outside the y-clipbox are filtered.

    bbU8* const pStackStart = (bbU8*)(pSort + 256); // 256 bytes
    bbU8* pStack            = pStackStart;
    ptGCEDGE* pEdge         = (ptGCEDGE*)(pStackStart + 512);     // 256*12=3kB
    i=0;
    for(;;)
    {
        for(;;)
        {
            const bbUINT e = (bbUINT)pSort[i];
            if (!(e&0xFFU)) break;
            *(pStack++)=(bbU8)i;
            pSort[i] = (bbU16)(e&0xFF00U);
            i=e&0xFFU;
        }

        if (pPoints[i].y < mUnitClipMaxY)
        {
            const int il = ((i>0) ? i : pointcount)-1;
            int dy = (pPoints[il].y>> ptGCEIGHTY) - (pPoints[i].y >> ptGCEIGHTY);
            if ((pPoints[il].y >= mUnitClipMinY) && (dy>0))
            {
                pEdge->y  = (short) pPoints[i].y;
                pEdge->ye = (short) pPoints[il].y;
                pEdge->x  = (long) pPoints[i].x << bbDIVTABPREC;

                int dx = pPoints[il].x - pPoints[i].x;

                while (dy>=bbDIVTABSIZE) dy>>=1,dx>>=1;
                pEdge->sx = (long)bbgpDivTab[dy] * dx;

                bbUINT m = i;
                do
                {
                    if (++m >= pointcount) m=0;
                } while (pPoints[i].y == pPoints[m].y);

                if (pPoints[i].y > pPoints[m].y)
                {
                    pEdge->y = (short)(pPoints[i].y + (1<<ptGCEIGHTY));
                    pEdge->x += pEdge->sx;
                }

                pEdge++;
            }

            const int ir = ((i+1)<pointcount) ? (i+1) : 0;
            dy = (pPoints[ir].y>> ptGCEIGHTY) - (pPoints[i].y >> ptGCEIGHTY);
            if ((pPoints[ir].y >= mUnitClipMinY) && (dy>0))
            {
                pEdge->y  = (short) pPoints[i].y;
                pEdge->ye = (short) pPoints[ir].y;
                pEdge->x  = (long) pPoints[i].x << bbDIVTABPREC;

                int dx = pPoints[ir].x - pPoints[i].x;
                while (dy>=bbDIVTABSIZE) dy>>=1,dx>>=1;
                pEdge->sx = (long)bbgpDivTab[dy] * dx;

                if (pPoints[i].y > pPoints[ (bbUINT)pFrom[i] ].y)
                {
                    pEdge->y = (short)(pPoints[i].y + (1<<ptGCEIGHTY));
                    pEdge->x += pEdge->sx;
                }

                pEdge++;
            }
        }

        if ((bbUINT)pSort[i]&0xFF00U)
        {
            i=(bbUINT)pSort[i]>>8;
        }
        else
        {
            if (pStack == pStackStart) break;
            i=(bbUINT)*(--pStack);
        }
    }

    ptGCEDGE* const pEdges = (ptGCEDGE*)(pStackStart + 512);

    // xxx This tests if the poly is completely outside y-clipbox.
    // Better do this test in the very first y-enum loop, because
    // this saves the overhead of the second y-sort loop.
    if (pEdge == pEdges) return;

    bbUINT ae = 0;

    y = pEdges->y;

    // y-clipping for miny

    if (y < mUnitClipMinY)
    {
        y = mUnitClipMinY;
        i=0;
        int dy = y - pEdges->y;
        do
        {
            pEdges[i].x += (pEdges[i].sx * dy) >> ptGCEIGHTY;
            pEdges[i++].y = y;

        } while (((pEdges+i)<pEdge) && ((dy = y - (pEdges+i)->y) > 0));
    }
    bbASSERT(y < mUnitClipMaxY);

    // loop through scanlines

    y >>= ptGCEIGHTY;
    const bbUINT pitch = GetPitch();
    bbU8* pScr = GetBmp() + (long)y * pitch;

    const int maxy = mUnitClipMaxY >> ptGCEIGHTY;

    #if ptGCAAPREC == ptGCEIGHTX
    const int minx = ((pen & ptPENMASK_TYPE) != ptPEN_AA) ? (mUnitClipMinX>>ptGCEIGHTX) : mUnitClipMinX;
    const int maxx = ((pen & ptPENMASK_TYPE) != ptPEN_AA) ? ((mUnitClipMaxX>>ptGCEIGHTX)-1) : (mUnitClipMaxX-(1<<ptGCAAPREC));
    #elif (ptGCAAPREC > ptGCEIGHTX)
    const int minx = ((pen & ptPENMASK_TYPE) != ptPEN_AA) ? (mUnitClipMinX>>ptGCEIGHTX) : ((mUnitClipMinX<<(ptGCAAPREC-ptGCEIGHTX))-(1<<(ptGCAAPREC-ptGCEIGHTX)));
    const int maxx = ((pen & ptPENMASK_TYPE) != ptPEN_AA) ? ((mUnitClipMaxX>>ptGCEIGHTX)-1) : ((mUnitClipMaxX>>(ptGCAAPREC-ptGCEIGHTX))-(1<<(ptGCAAPREC-ptGCEIGHTX)));
    #else
    #error adjust shift
    #endif

    i = mpColLU[ pen &~ ptPENMASK ];

    const bbU8* const pAALU = mpAALU;
    const bbU8* const pAA = pAALU + 256 + ((bbUINT)pAALU[i]<<(ptGCAASIZE_LOG2+ptGCAAPREC)) + (((pen & ptPENMASK_TYPE) == ptPEN_TRANS) ? (pen>>ptPENBITPOS_OPT2) : 0);

    i = i | (i<<8);
    const bbU32 col32 = (bbU32)i | ((bbU32)i<<16);

    i=0;
    for(;;)
    {
        // activate all edges that start in this scanline

        while (((pEdges+i)<pEdge) && (y >= ((pEdges+i)->y) >> ptGCEIGHTY))
        {
            pAE[ae++] = i++;
        }

        if (ae==0) break;

if (!((ae>0) && (ae<=pointcount) && ((ae&1)==0)))
break;
        bbASSERT((ae>0) && (ae<=pointcount) && ((ae&1)==0));

        // x-enum loop. build a bintree for all x-coords in this scanline.

        *pSort = 0U;
        bbUINT j = 1;
        do
        {
            const long x = pEdges[pAE[j]].x;

            bbUINT ko;
            bbUINT k=0;
            do
            {
                ko=k;
                if (x < pEdges[pAE[k]].x) k=pSort[k]&0xFFU; else k=pSort[k]>>8;
            } while (k);

            pSort[ko] |= (x < pEdges[pAE[k]].x) ? j : (j<<8);
            pSort[j] = 0U;

        } while (++j < ae);

        // x-sort and plotloop

        pStack = pStackStart;
        j=0;

        switch ((pen & ptPENMASK_TYPE)>>ptPENBITPOS_TYPE)
        {
        case ptPEN_COL>>ptPENBITPOS_TYPE:
        for(;;)
        {
            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x0 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX));

            bbASSERT( ((bbUINT)pSort[j]&0xFF00U) || (pStack != pStackStart) ); // this never happens as ae is even
            j = ((bbUINT)pSort[j]&0xFF00U) ? ((bbUINT)pSort[j]>>8) : (bbUINT)*(--pStack);

            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x1 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX));

            if (x0 < minx) x0 = minx;
            if (x1 > maxx) x1 = maxx;

            while ((x0&3)&&(x0<=x1)) pScr[x0++]=(bbU8)col32;
            const int xe = x1 &~ 3U;
            while (x0<xe) { *(bbU32*)(pScr+x0)=col32; x0+=4; }
            while (x0<=x1) pScr[x0++]=(bbU8)col32;

            if ((bbUINT)pSort[j]&0xFF00U)
            {
                j=(bbUINT)pSort[j]>>8;
            }
            else
            {
                if (pStack == pStackStart) break;
                j=(bbUINT)*(--pStack);
            }
        }
        break;

        case ptPEN_TRANS>>ptPENBITPOS_TYPE:
        for(;;)
        {
            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x0 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX));

            bbASSERT( ((bbUINT)pSort[j]&0xFF00U) || (pStack != pStackStart) ); // this never happens as ae is even
            j = ((bbUINT)pSort[j]&0xFF00U) ? ((bbUINT)pSort[j]>>8) : (bbUINT)*(--pStack);

            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x1 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX));

            if (x0 < minx) x0 = minx;
            if (x1 > maxx) x1 = maxx;

            while (x0<=x1) pScr[x0++]=pAA[(bbUINT)pAALU[pScr[x0]]<<(ptGCAAPREC)];

            if ((bbUINT)pSort[j]&0xFF00U)
            {
                j=(bbUINT)pSort[j]>>8;
            }
            else
            {
                if (pStack == pStackStart) break;
                j=(bbUINT)*(--pStack);
            }
        }
        break;

        case ptPEN_AA>>ptPENBITPOS_TYPE:
        for(;;)
        {
            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x0 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX-ptGCAAPREC));
if(!( ((bbUINT)pSort[j]&0xFF00U) || (pStack != pStackStart) ))
    return;

            bbASSERT( ((bbUINT)pSort[j]&0xFF00U) || (pStack != pStackStart) ); // this never happens as ae is even
            j = ((bbUINT)pSort[j]&0xFF00U) ? ((bbUINT)pSort[j]>>8) : (bbUINT)*(--pStack);

            for(;;)
            {
                const bbUINT f = (bbUINT)pSort[j];
                if (!(f&0xFFU)) break;
                *(pStack++)=(bbU8)j;
                pSort[j] = (bbU16)(f&0xFF00U);
                j=f&0xFFU;
            }

            int x1 = (int)(pEdges[pAE[j]].x >> (bbDIVTABPREC+ptGCEIGHTX-ptGCAAPREC));

            if ((x0<=maxx) && (x1>=minx))
            {
                if (x0 < minx)
                {
                    x0 = minx>>ptGCAAPREC;
                }
                else
                {
                    pScr[x0>>ptGCAAPREC] = pAA[ ((bbUINT)pAALU[pScr[x0>>ptGCAAPREC]]<<ptGCAAPREC) + (x0 & ((1<<ptGCAAPREC)-1)) ];
                    x0 = (x0>>ptGCAAPREC) + 1;
                }

                if (x1 > maxx)
                {
                    x1 = maxx;
                }
                else
                {
                    bbU8* const pTmp = pScr + 1 + (x1>>ptGCAAPREC);
                    *pTmp = pAA[ ((bbUINT)pAALU[*pTmp]<<ptGCAAPREC) + ((x1 & ((1<<ptGCAAPREC)-1))^((1<<ptGCAAPREC)-1))];
                }

                const int xe = x1>>ptGCAAPREC;

                while ((x0&3)&&(x0<=xe)) pScr[x0++]=(bbU8)col32;
                const int xe4 = xe &~ 3U;
                while (x0<xe4) { *(bbU32*)(pScr+x0)=col32; x0+=4; }
                while (x0<=xe) pScr[x0++]=(bbU8)col32;
            }

            if ((bbUINT)pSort[j]&0xFF00U)
            {
                j=(bbUINT)pSort[j]>>8;
            }
            else
            {
                if (pStack == pStackStart) break;
                j=(bbUINT)*(--pStack);
            }
        }
        break;
        }

        // increase scanpositions in all active edges and deactivate finished edges

        pScr += pitch;
        if (++y >= maxy) break;

        j=0;
        do
        {
            const bbUINT ia = (bbUINT) pAE[j];

            if (y <= (pEdges[ia].ye >> ptGCEIGHTY))
            {
                pEdges[ia].x += pEdges[ia].sx;
                j++;
            }
            else
            {
                pAE[j] = pAE[--ae];
            }
        } while (j<ae);
    }

    #undef pDir
}

void ptGC8::FillCircle(int x, int y, bbUINT rad, const ptPEN pen)
{
    Point(x,y,pen);
    VLine(x,y-rad,2*rad,pen);
    HLine(x-rad,y,2*rad,pen);
}

bbUINT ptGC8::Text(int x, int y, const bbCHAR* pMarkup, bbUINT fgcol, ptPEN bgpen, bbUINT const font)
{
    if ((x >= mUnitClipMaxX) || (y >= mUnitClipMaxY)) return 0;

    ptFont* pFont = ptgFontMan.mFonts[font];

    if ((int)(y + (pFont->GetHeight()<<ptGCEIGHTY)) <= mUnitClipMinY) return 0;

    const bbUINT pitch = GetPitch();

    bbU8* pDst = GetBmp() + (long)(y>>ptGCEIGHTY) * pitch;

    bbUINT clipright;
    bbU8* const pDst_clipminx = pDst + (mUnitClipMinX>>ptGCEIGHTX);
    bbU8* const pDst_clipmaxx = pDst + (mUnitClipMaxX>>ptGCEIGHTX);
    const int cliptop = (y < mUnitClipMinY) ? ((mUnitClipMinY - (y &~ ((1<<ptGCEIGHTY)-1)))>>ptGCEIGHTY) : 0;
    pDst += (x>>ptGCEIGHTX);
    bbU8* const pDst_start = pDst;

    const bbU8* const pAALU = mpAALU;
    const bbU8* pAA = pAALU + 256 + (bgpen>>ptPENBITPOS_OPT2) + ((bbUINT)pAALU[mpColLU[bgpen &~ ptPENMASK]]<<(ptGCAASIZE_LOG2+ptGCAAPREC));

    for(;;)
    {
        bbCHARCP cp;
        bbCP_NEXT_PTR(pMarkup, cp)

        if (cp == 0)
            return ((bbUINT)(bbUPTR)pDst-(bbUINT)(bbUPTR)pDst_start) << ptGCEIGHTX;

        if (cp >= 0x110000UL)
            cp = pFont->mUkCP; // outside UNICODE codepage

        if (cp == 27)
        {
            cp = *(pMarkup++);

            bbASSERT(cp <= 5);

            switch (cp)
            {
            case 1:
                cp = 0;
                break;
            case 2:
                fgcol = *(pMarkup++);
                continue;
            case 3:
                bgpen = (bgpen & ptPENMASK) | ((bbUINT)*(pMarkup++) & 0xFFU);
                pAA = pAALU + 256 + (bgpen>>ptPENBITPOS_OPT2) + ((bbUINT)pAALU[mpColLU[bgpen &~ ptPENMASK]]<<(ptGCAASIZE_LOG2+ptGCAAPREC));
                continue;
            case 4:
                pFont = ptgFontMan.mFonts[ *(pMarkup++) ];
                continue;
            case 5:
                #if bbSIZEOF_CHARCP == bbSIZEOF_CHAR
                cp = *(pMarkup++);
                #elif (bbSIZEOF_CHARCP == bbSIZEOF_CHAR*2)
                cp = (bbCHARCP)pMarkup[0] | ((bbCHARCP)pMarkup[1]<<(8*bbSIZEOF_CHAR));
                pMarkup += 2;
                #elif (bbSIZEOF_CHARCP == 4) && (bbSIZEOF_CHAR == 1)
                cp = (bbCHARCP)pMarkup[0] | ((bbCHARCP)pMarkup[1]<<8) | ((bbCHARCP)pMarkup[2]<<16) | ((bbCHARCP)pMarkup[3]<<24);
                pMarkup += 4;
                #else
                #error not implemented
                #endif
                break;
            case 27:
                break;
            }
        }

        const bbU8* pSrc = pFont->GetBankNoReplace(cp);
        bbU32 tmp;

        if (!pSrc || ((tmp = *((bbU32*)pSrc + (cp & (ptFONT_BANKSIZE-1))))==0)) // glyph not existent
        {
            // if bbCE is Unicode, and ptFont is monospace, preserve the widechar property of the unknown character
            #if (bbCPG == bbCPG_UNICODE) || (bbCPG == bbCPG_UCS)
            if (pFont->GetWidth() && bbCpgUnicode_IsWide(cp))
            {
                cp = pFont->mUkCPW;
                pSrc = pFont->mpBankUkCPW;
            }
            else
            #endif
            {
                cp = pFont->mUkCP;
                pSrc = pFont->mpBankUkCP;
            }

            tmp = *((bbU32*)pSrc + (cp & (ptFONT_BANKSIZE-1)));
        }

        pSrc = pSrc + (tmp &~ 0xFF000000UL);

        bbU8* pTmpy = pDst;
        pDst += (bbUINT)(tmp >> 26);

        // clip max x
        if (pDst > pDst_clipmaxx)
        {
            if (pTmpy >= pDst_clipmaxx) return ((bbUINT)(bbUPTR)pDst_clipmaxx-(bbUINT)(bbUPTR)pDst_start) << ptGCEIGHTX; // chars completely right of clip box

            clipright = (bbUINT)(tmp >> 26);
            clipright += 7;
            clipright >>= 3;
            tmp -= ((bbU32)(bbUPTR)pDst - (bbU32)(bbUPTR)pDst_clipmaxx) << 26;
            clipright -= (bbUINT)(tmp >> (26+3));
            if (tmp & (7U<<26)) clipright--;
        }
        else clipright = 0;

        int yctr = (int) pFont->GetHeight();

        // clip max y
        if ((y + (yctr<<ptGCEIGHTY)) > mUnitClipMaxY)
        {
            yctr = (mUnitClipMaxY - y)>>ptGCEIGHTY;
        }

        // clip min y
        if (cliptop)
        {
            yctr -= cliptop;
            int charpitch = (int)(tmp >> (26+3)); if (tmp & (7<<26)) charpitch++;
            pSrc += cliptop * charpitch;
        }

        // clip min x
        if (pTmpy < pDst_clipminx)
        {
            if (pDst <= pDst_clipminx) continue; // char completely left of clip box

            const bbUINT clipleft = ((bbUINT)(bbUPTR)pDst_clipminx - (bbUINT)(bbUPTR)pTmpy);
            pTmpy = pDst_clipminx;

            #if bbCPU!=bbCPU_ARM
            if (cliptop)
            #endif
                pTmpy += pitch * cliptop;

            if ((bgpen & ptPENMASK_TYPE) != ptPEN_TRANS)
            {
                do
                {
                    bbU8* pTmp = pTmpy;
                    pTmpy += pitch;

                    bbUINT bits;
                    pSrc += clipleft>>3;
                    if (clipleft & 7) bits = (bbUINT) *(pSrc++) >> (clipleft & 7);

                    for (bbUINT width = clipleft; width < (bbUINT)(tmp >> 26); ++width)
                    {
                        if ((width & 7) == 0) bits = (bbUINT) *(pSrc++);
                        *(pTmp++) = (bits & 1) ? fgcol : bgpen;
                        bits >>= 1;
                    }

                    pSrc += clipright;

                } while (--yctr > 0);
            }
            else
            {
                do
                {
                    bbU8* pTmp = pTmpy;
                    pTmpy += pitch;

                    bbUINT bits;
                    pSrc += clipleft>>3;
                    if (clipleft & 7) bits = (bbUINT) *(pSrc++) >> (clipleft & 7);

                    for (bbUINT width = clipleft; width < (bbUINT)(tmp >> 26); ++width)
                    {
                        if ((width & 7) == 0) bits = (bbUINT) *(pSrc++);
                        *(pTmp++) = (bits & 1) ? fgcol : pAA[(bbUINT)pAALU[*pTmp]<<ptGCAAPREC];
                        bits >>= 1;
                    }

                    pSrc += clipright;

                } while (--yctr > 0);
            }

            continue;
        }

        #if bbCPU!=bbCPU_ARM
        if (cliptop)
        #endif
            pTmpy += pitch * cliptop;

        if ((bgpen & ptPENMASK_TYPE) != ptPEN_TRANS)
        {
            do
            {
                bbU8* pTmp = pTmpy;
                pTmpy += pitch;
                bbUINT width = (bbUINT)(tmp >> 26);

                while (width >= 8)
                {
                    const bbUINT bits = (bbUINT) *(pSrc++);
                    *(pTmp+0) = (bits & 0x01) ? fgcol : bgpen;
                    *(pTmp+1) = (bits & 0x02) ? fgcol : bgpen;
                    *(pTmp+2) = (bits & 0x04) ? fgcol : bgpen;
                    *(pTmp+3) = (bits & 0x08) ? fgcol : bgpen;
                    *(pTmp+4) = (bits & 0x10) ? fgcol : bgpen;
                    *(pTmp+5) = (bits & 0x20) ? fgcol : bgpen;
                    *(pTmp+6) = (bits & 0x40) ? fgcol : bgpen;
                    *(pTmp+7) = (bits & 0x80) ? fgcol : bgpen;
                    pTmp+=8;
                    width-=8;
                }

                if (width)
                {
                    bbUINT bits = (bbUINT) *(pSrc++);
                    do
                    {
                        *(pTmp++) = (bits & 1) ? fgcol : bgpen;
                        bits >>= 1;
                    } while (--width);
                }

                pSrc += clipright;

            } while (--yctr > 0);
        }
        else
        {
            do
            {
                bbU8* pTmp = pTmpy;
                pTmpy += pitch;
                bbUINT width = (bbUINT)(tmp >> 26);

                while (width >= 8)
                {
                    const bbUINT bits = (bbUINT) *(pSrc++);
                    *(pTmp+0) = (bits & 0x01) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+0)]<<ptGCAAPREC];
                    *(pTmp+1) = (bits & 0x02) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+1)]<<ptGCAAPREC];
                    *(pTmp+2) = (bits & 0x04) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+2)]<<ptGCAAPREC];
                    *(pTmp+3) = (bits & 0x08) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+3)]<<ptGCAAPREC];
                    *(pTmp+4) = (bits & 0x10) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+4)]<<ptGCAAPREC];
                    *(pTmp+5) = (bits & 0x20) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+5)]<<ptGCAAPREC];
                    *(pTmp+6) = (bits & 0x40) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+6)]<<ptGCAAPREC];
                    *(pTmp+7) = (bits & 0x80) ? fgcol : pAA[(bbUINT)pAALU[*(pTmp+7)]<<ptGCAAPREC];
                    pTmp+=8;
                    width-=8;
                }

                if (width)
                {
                    bbUINT bits = (bbUINT) *(pSrc++);
                    do
                    {
                        *(pTmp++) = (bits & 1) ? fgcol : pAA[(bbUINT)pAALU[*pTmp]<<ptGCAAPREC];
                        bits >>= 1;
                    } while (--width);
                }

                pSrc += clipright;

            } while (--yctr > 0);
        }
    }
}

void ptGC8::Sprite(int x, int y, const ptSprite* const pSprite)
{
    int          d;
    int          lpy      = pSprite->height;
    bbU8*        pSpr     = pSprite->pData;
    int          width    = pSprite->width;
    bbUINT const sprpitch = pSprite->stride;
    bbUINT const bmppitch = mPixPitch;
    bbU8*        pBmp     = mpBmp + (int)bmppitch*(y>>ptGCEIGHTY) + (x>>ptGCEIGHTX);

    //
    // Clip on top
    //
    if ((d = (mUnitClipMinY - y)) > 0)
    {
        if ((lpy -= d) <= 0) // completely clipped above
            return;
        y += d;
        pBmp += bmppitch * (d>>ptGCEIGHTY);
        pSpr += sprpitch * (d>>ptGCEIGHTY);
    }

    //
    // Clip on bottom
    //
    if ((d = ((y+lpy) - mUnitClipMaxY)) > 0)
    {
        if ((lpy -= d) <= 0) // completely clipped on bottom
            return;
    }

    switch (pSprite->colfmt)
    {
    case ptCOLFMT_4BPP:
        //xxx clipping
        bbASSERT((width&1)==0);
        do
        {
            int lpx = width>>1;
            do
            {
                int r0 = *(pSpr++);
                *(pBmp++) = r0 & 0xF;
                *(pBmp++) = r0 >> 4;
            } while (--lpx);

            pSpr+=sprpitch-(width>>1);
            pBmp+=bmppitch-width;
        } while (--lpy);

        break;

    case ptCOLFMT_8BPP:

        //
        // Clip left
        //
        if ((d = (mUnitClipMinX - x)) > 0)
        {
            if ((width -= d) <= 0)
                return;
            x += d;
            pBmp += (d>>ptGCEIGHTX);
            pSpr += (d>>ptGCEIGHTX);
        }

        //
        // Clip right
        //
        if ((d = ((x+width) - mUnitClipMaxX)) > 0)
        {
            if ((width -= d) <= 0)
                return;
        }

        do
        {
            bbMemMove(pBmp, pSpr, width);
            pSpr+=sprpitch;
            pBmp+=bmppitch;
        } while (--lpy);

        break;
    }
}

#endif /* ptUSE_GC8 == 1 */

