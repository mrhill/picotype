#include "ptGC.h"
#include "ptPalMan.h"
#include "ptFontMan.h"
#include <babel/str.h>

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

    bbST16LE(&text[0], 0x0500U);
    bbST32LE(&text[2], cp);
    bbST16LE(&text[6], 0x0100U);

    #else
    #error not implemented
    #endif

    return Text(x, y, text, fgcol, bgpen, font);
}

bbUINT ptGC::Text(int x, int y, const bbCHAR* pText, bbUINT fgcol, ptPEN bgpen, bbUINT const font)
{
    bbUINT i=0;
    bbU32 cp;
    bbUINT fontIdx = 0, fgcolIdx = 0, bgpenIdx = 0;
    bbUINT fontCnt = 1, fgcolCnt = 1, bgpenCnt = 1;
    ptMarkupInfo info;
    bbU32 text[256];

    info.mpFont[0] = ptgFontMan.mFonts[font];
    info.mBGPen[0] = bgpen;
    info.mFGCol[0] = fgcol;

    do
    {
        bbCP_NEXT_PTR(pText, cp);

        if (cp >= 0x110000UL)
            cp = info.mpFont[fontIdx]->mUkCP; // outside UNICODE codepage

        if (cp == 0)
        {
            cp = *(pText++);

            switch (cp)
            {
            case 1: 
                goto ptGC_Text_out;
            case 2:
                cp = *pText++;

                for (fgcolIdx=0; fgcolIdx<fgcolCnt; fgcolIdx++)
                    if (cp == info.mFGCol[fgcolIdx])
                        continue;

                if (fgcolCnt < bbARRSIZE(info.mFGCol))
                {
                    fgcolIdx = fgcolCnt++;
                    info.mFGCol[fgcolIdx] = (bbU8)cp;
                    continue;
                }

                fgcolIdx=0;
                continue;
            case 3:
                cp = (bgpen & ptPENMASK) | ((bbUINT)*pText++ & 0xFFU);
                
                for (bgpenIdx=0; bgpenIdx<bgpenCnt; bgpenIdx++)
                    if (cp == info.mBGPen[bgpenIdx])
                        continue;

                if (bgpenCnt < bbARRSIZE(info.mBGPen))
                {
                    bgpenIdx = bgpenCnt++;
                    info.mBGPen[bgpenIdx] = (bbU16)cp;
                    continue;
                }

                bgpenIdx=0;
                continue;
            case 4:
                cp = *pText++;
                
                for (fontIdx=0; fontIdx<fontCnt; fontIdx++)
                    if (ptgFontMan.mFonts[cp] == info.mpFont[fontIdx])
                        continue;

                if (fontCnt < bbARRSIZE(info.mpFont))
                {
                    fontIdx = fontCnt++;
                    info.mpFont[fontIdx] = ptgFontMan.mFonts[cp];
                    continue;
                }

                fontIdx=0;
                continue;
            case 5:
                #if bbSIZEOF_CHARCP == bbSIZEOF_CHAR
                cp = *pText++;
                #elif (bbSIZEOF_CHARCP == bbSIZEOF_CHAR*2)
                cp = (bbU32)pText[0] | ((bbCHARCP)pText[1]<<(8*bbSIZEOF_CHAR));
                pText += 2;
                #elif (bbSIZEOF_CHARCP == 4) && (bbSIZEOF_CHAR == 1)
                cp = bbLD32LE(pText);
                pText += 4;
                #endif
                break;
            }
        }

        text[i++] = cp | (fontIdx << 22) | (fgcolIdx << 28) | (bgpenIdx << 29);

        if (bbCpgUnicode_IsWide(cp))
            text[i++] = 0x3FFFFEUL;

    } while (i+3 <= bbARRSIZE(text));

    ptGC_Text_out:

    text[i] = 0xFFFFFFFFUL;
    return MarkupText(x, y, text, &info, 0);
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

