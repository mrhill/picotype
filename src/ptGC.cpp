#include "ptGC.h"
#include "ptPalMan.h"
#include "ptFontMan.h"
#include "babel/str.h"

static void*  ptgpScratch = NULL;     //!< Scratch space of size ptGCSCRATCHSIZE, can be shared by all instances
static bbUINT ptgScratchRefCt = 0;    //!< ptgpScratch reference count

void* ptScratchAlloc()
{
    if (ptgScratchRefCt == 0)
    {
        if ((ptgpScratch = bbMemAlloc(ptGCSCRATCHSIZE)) == NULL)
            return NULL;
    }

    ptgScratchRefCt++;

    return ptgpScratch;
}

void ptScratchFree(void** ppScratch)
{
    if (*ppScratch)
    {
        bbASSERT(ptgScratchRefCt);
        bbASSERT(*ppScratch == ptgpScratch);

        if (--ptgScratchRefCt == 0)
        {
            bbMemFreeNull(&ptgpScratch);
        }

        *ppScratch = NULL;
    }
}

bbU32 ptRGBADistA(const bbU32 a, const bbU32 b)
{
    long ca = a>>24; 
    long cb = b>>16;
    ca = ca-cb;
    return ca*ca;
}

bbU32 ptRGBDist(const bbU32 a, const bbU32 b)
{
    long ca, cb;

    ca = a & 0xFFU; 
    cb = b & 0xFFU;
    ca = ca-cb;
    long dist = ca*ca;

    ca = (a>>8) & 0xFFU; 
    cb = (b>>8) & 0xFFU;
    cb = ca-cb;
    dist += cb*cb;

    ca = (a>>16) & 0xFFU; 
    cb = (b>>16) & 0xFFU;
    ca = ca-cb;
    return (bbU32)(dist + ca*ca);
}

bbU32 ptGetWeightedRGB(const bbU32 a, const bbU32 b, const bbU32 weight)
{
    bbU32 rgb;
    bbU32 ca, cb;

    ca = a & 0xFF;
    cb = b & 0xFF;
    cb = (cb-ca) * weight;
    if (cb & 0x8000) cb+=0x8000;
    rgb = ca + (cb >> 16);

    ca = (a>>8) & 0xFF;
    cb = (b>>8) & 0xFF;
    cb = (cb-ca) * weight;
    if (cb & 0x8000) cb+=0x8000;
    rgb |= ((ca + (cb >> 16)) & 0xFF)<<8;

    ca = (a>>16) & 0xFF;
    cb = (b>>16) & 0xFF;
    cb = (cb-ca) * weight;
    if (cb & 0x8000) cb+=0x8000;

    return rgb | (((ca + (cb >> 16)) & 0xFF)<<16);
}

bbUINT ptRGBPalMatch(const bbU32* const pRGB, bbUINT const palsize, const bbU32 rgb)
{
    bbU32  bestdist = ptRGBDist(pRGB[0], rgb);
    bbUINT bestidx = 0;

    for (bbUINT i=1; i<palsize; ++i)
    {
        const bbU32 currgb = pRGB[i];
        const bbU32 dist = ptRGBDist(currgb, rgb);
        if (dist < bestdist)
        {
            bestdist = dist;
            bestidx = i;
        }
    }

    return bestidx;
}

ptPal* ptGC::SetLogPal(ptPal* const pLogRGB)
{
    ptPal* const pOldPal = mpLogPal;
    mpLogPal = pLogRGB;
    return pOldPal;
}

void ptGC::ResetClipBox()
{
    SetClipBox(0, 0, GetWidth(), GetHeight());
}

void ptGC::Box(const int x, const int y, const bbUINT width, const bbUINT height, const bbUINT col)
{
    HLine(x, y, width, col);
    HLine(x, y+height-(1 << ptGCEIGHTY), width, col);
    VLine(x, y, height, col);
    VLine(x+width-(1 << ptGCEIGHTX), y, height, col);
}

void ptGC::LineString(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen)
{
    if (points==0) return;
    for (bbUINT i=1; i<points; ++i)
    {
        Line( pPoly[i-1].x, pPoly[i-1].y, pPoly[i].x, pPoly[i].y, pen);
    }
}

void ptGC::LinearRing(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen)
{
    if (points==0) return;
    for (bbUINT i=1; i<points; ++i)
    {
        Line( pPoly[i-1].x, pPoly[i-1].y, pPoly[i].x, pPoly[i].y, pen);
    }
    Line( pPoly[points-1].x, pPoly[points-1].y, pPoly[0].x, pPoly[0].y, pen);
}

bbUINT ptGC::Char(const int x, const int y, const bbCHARCP cp, bbUINT const fgcol, ptPEN const bgpen, const bbUINT font)
{
    bbCHAR text[sizeof(bbCHARCP)/sizeof(bbCHAR) + 4];

    #if bbSIZEOF_CHARCP == bbSIZEOF_CHAR

    text[0] = 0;
    text[1] = 5;
    text[2] = (bbCHAR)cp;
    text[3] = 0;
    text[4] = 1;

    #elif (bbSIZEOF_CHARCP == bbSIZEOF_CHAR*2)

    #if bbCPUE == bbCPUE_LE
    *(bbCHARCP*)&text[0] = 0 | (5 << (8*bbSIZEOF_CHAR));
    *(bbCHARCP*)&text[2] = cp;
    *(bbCHARCP*)&text[4] = 0 | (1 << (8*bbSIZEOF_CHAR));
    #else
    text[0] = 0;
    text[1] = 5;
    text[2] = (bbCHAR)cp;
    text[3] = (bbCHAR)((bbU32)cp >> (8*bbSIZEOF_CHAR));
    text[4] = 0;
    text[5] = 1;
    #endif

    #elif (bbSIZEOF_CHARCP == 4) && (bbSIZEOF_CHAR == 1)

    #if bbCPUE == bbCPUE_LE
    *(bbU16*)&text[0] = (bbU16)0x0500U;
    *(bbU16*)&text[2] = (bbU16)cp;
    *(bbU16*)&text[4] = (bbU16)((bbU32)cp>>16);
    #else
    text[0] = 0;
    text[1] = 5;
    text[2] = (bbCHAR)cp;
    text[3] = (bbCHAR)((bbU32)cp >> 8);
    text[4] = (bbCHAR)((bbU32)cp >> 16);
    text[5] = (bbCHAR)((bbU32)cp >> 24);
    text[6] = 0;
    text[7] = 1;
    #endif
    #else
    #error not implemented
    #endif

    return Text(x, y, text, fgcol, bgpen, font);
}

void ptGC::MarkupTextBox(ptTextBox* const pTextbox)
{
    #if ptTEXTBOX_HEAPH == 1
    int linecount = (int) pTextbox->linecount;
    #endif

    ptRect clip;
    GetClipBox(&clip);

    if ((pTextbox->unit_x >= clip.right) ||
        ((int)(pTextbox->unit_x + pTextbox->unit_width) < clip.left) ||
        (pTextbox->unit_y >= clip.bottom) ||
        ((int)(pTextbox->unit_y + pTextbox->unit_height) < clip.top) )
    {
        return;
    }

    int x = pTextbox->unit_x - pTextbox->unit_scrollx;
    int y = pTextbox->unit_y - pTextbox->unit_scrolly;

    ptRect clipnew;

    clipnew.left   = (pTextbox->unit_x >= clip.left) ? pTextbox->unit_x : clip.left;
    clipnew.top    = (pTextbox->unit_y >= clip.top) ? pTextbox->unit_y : clip.top;
    clipnew.right  = pTextbox->unit_x + pTextbox->unit_width;
    if (clipnew.right > clip.right) clipnew.right = clip.right;
    clipnew.bottom = pTextbox->unit_y + pTextbox->unit_height;
    if (clipnew.bottom > clip.bottom) clipnew.bottom = clip.bottom;

    SetClipBox(&clipnew);

    #if ptTEXTBOX_HEAPH == 1
    const bbHEAPH* pLines = pTextbox->pLines;
    #else
    const bbCHAR* const* pLines = pTextbox->pLines;
    #endif

    const bbUINT fontheight = ptgFontMan.mFonts[ pTextbox->font ]->GetHeight() << ptGCEIGHTX;
    const int fillx = bbMAX(x, pTextbox->unit_x);

    if (pTextbox->unit_scrollx < 0)
    {
        FillBox(pTextbox->unit_x, pTextbox->unit_y, -pTextbox->unit_scrollx, pTextbox->unit_height, pTextbox->bgpen);
    }
    if (pTextbox->unit_scrolly < 0)
    {
        FillBox(fillx, pTextbox->unit_y, pTextbox->unit_width, -pTextbox->unit_scrolly, pTextbox->bgpen);
    }

    const int lineheight = fontheight + pTextbox->unit_linespace;
    for(;;)
    {
        if ((clipnew.top - y) < lineheight) break;

        #if ptTEXTBOX_HEAPH == 1
        if ((--linecount<0) || (y >= clipnew.bottom)) goto ptGC_MarkupTextBox_skip;
        pLines++;
        #else
        const bbCHAR* const pLine = *(pLines++);
        if ((!pLine) || (y >= clipnew.bottom)) goto ptGC_MarkupTextBox_skip;
        #endif

        y += lineheight;
    }

    for(;;)
    {
        #if ptTEXTBOX_HEAPH == 1
        if ((--linecount<0) || (y >= clipnew.bottom)) break;
        const bbCHAR* const pLine = (const bbCHAR*) bbHeapGetPtr(*(pLines++));
        #else
        const bbCHAR* const pLine = *(pLines++);
        if ((!pLine) || (y >= clipnew.bottom)) break;
        #endif

        const bbUINT plotwidth = Text(x, y, pLine, pTextbox->textfgcol, pTextbox->textbgpen, pTextbox->font);
        FillBox(x+plotwidth, y, clipnew.right-x, fontheight, pTextbox->bgpen);
        y += fontheight;

        if (pTextbox->unit_linespace > 0)
        {
            FillBox(fillx, y, pTextbox->unit_width, pTextbox->unit_linespace, pTextbox->bgpen);
            y += pTextbox->unit_linespace;
        }
    }

    ptGC_MarkupTextBox_skip:

    FillBox(fillx, bbMAX(y, pTextbox->unit_y), pTextbox->unit_width, pTextbox->unit_height, pTextbox->bgpen);

    SetClipBox(clip.left, clip.top, clip.right, clip.bottom);
}

