#include "ptGCGDI.h"

#if ptUSE_GCGDI == 1

#include "ptPalMan.h"
#include "ptFontMan.h"
#include "ptSprite.h"
#include <babel/str.h>

ptGCGDI::ptGCGDI()
{
    mhDC = NULL;
    mpBMI = NULL;
    mhMask = NULL;
    mpMask = NULL;
    mpLineCache = NULL;
    mLineCacheSize = 0;
    mCachedPal = 0;
    mMaskWidth = mMaskHeight = 0;
}

ptGCGDI::~ptGCGDI()
{
    Destroy();
}

bbERR ptGCGDI::EnsureMaskBmp(bbUINT const width, bbUINT const height)
{
    if ((width <= mMaskWidth) && (height <= mMaskHeight))
        return bbEOK;

    DeleteObject(mhMask);
    mhMask = NULL;

    if (width > mMaskWidth)
        mMaskWidth = width;

    if (height > mMaskHeight)
        mMaskHeight = height;

    mpBMI->bmiHeader.biWidth = (LONG)(mMaskWidth + 3) &~ 3;
    mpBMI->bmiHeader.biHeight = -(int)mMaskHeight;
    mpBMI->bmiHeader.biCompression = BI_RGB;
    mpBMI->bmiHeader.biBitCount = 1;

    if ((mhMask = CreateDIBSection(mhDC, mpBMI, DIB_RGB_COLORS, (void**)&mpMask, NULL, 0)) == NULL)
    {
        mMaskWidth = mMaskHeight = 0;
        return bbErrSet(bbESYS);
    }

    return bbEOK;
}

bbERR ptGCGDI::Init()
{
    if ((mpBMI = (BITMAPINFO*) bbMemAlloc(sizeof(BITMAPINFOHEADER) + 256*4)) == NULL)
        return bbELAST;

    bbMemClear(mpBMI, sizeof(BITMAPINFOHEADER));
    mpBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    mpBMI->bmiHeader.biPlanes = 1;

    mhDotPen = CreatePen(PS_DOT, 1,RGB(0, 0, 0));
    if (!mhDotPen)
        return bbErrSet(bbESYS);

    return bbEOK;
}

void ptGCGDI::Destroy()
{
    DeleteObject(mhDotPen);
    DeleteObject(mhMask);
    mhMask = NULL;
    bbMemFreeNull((void**)&mpLineCache);
    bbMemFreeNull((void**)&mpBMI);
}

void ptGCGDI::AttachDC(HDC const hDC)
{
    mhDC = hDC;
    mWidth = ::GetDeviceCaps(hDC, HORZRES) << ptGCEIGHTX;
    mHeight = ::GetDeviceCaps(hDC, VERTRES) << ptGCEIGHTY;
}

bbUINT ptGCGDI::GetWidth() const
{
    return mWidth;
}

bbUINT ptGCGDI::GetHeight() const
{
    return mHeight;
}

void ptGCGDI::SetClipBox(const int clipminx, const int clipminy, const int clipmaxx, const int clipmaxy)
{
    HRGN hrgn = CreateRectRgn(clipminx, clipminy, clipmaxx, clipmaxy); 
    SelectClipRgn(mhDC, hrgn);
    DeleteObject(hrgn);
}

void ptGCGDI::GetClipBox(ptRect* const pRect)
{
    const int ret = ::GetClipBox(mhDC, (LPRECT)pRect);

    if ((ret == NULLREGION) || (ret == ERROR))
    {
        pRect->left = pRect->top = 0;
        pRect->right = GetWidth();
        pRect->bottom = GetHeight();
    }
}

void ptGCGDI::Clear(const bbUINT col)
{
    FillBox(0, 0, GetWidth(), GetHeight(), col);
}

void ptGCGDI::Point(const int x, const int y, const bbUINT col)
{
    SetPixel(mhDC, x>>ptGCEIGHTX, y>>ptGCEIGHTY, mpLogPal->mpRGB[col] & 0xFFFFFFUL);
}

void ptGCGDI::HLine(int x, int y, bbUINT width, bbUINT col)
{
    Line(x, y, x+width, y, col);
}

void ptGCGDI::VLine(int x, int y, bbUINT height, bbUINT col)
{
    Line(x, y, x, y+height, col);
}

static const int gROPtoR2[] = { R2_NOT, R2_MERGEPEN };

void ptGCGDI::FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen)
{
    const RECT rect = { x>>ptGCEIGHTX, y>>ptGCEIGHTX, (x+width)>>ptGCEIGHTX, (y+height)>>ptGCEIGHTY };

    if ((pen & ptPEN_ROP) && ((pen>>ptPENBITPOS_OPT2) == ptROP_NOT))
    {
        InvertRect(mhDC, &rect);
    }
    else
    {
        SetDCBrushColor(mhDC, mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
        FillRect(mhDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
    }
}

void ptGCGDI::Line(int x1, int y1, int x2, int y2, const ptPEN pen)
{
    int r2 = 0;
    if (pen & ptPEN_ROP)
        r2 = SetROP2(mhDC, gROPtoR2[pen >> ptPENBITPOS_OPT2]);

    if (pen & ptPEN_PAT)
    {
        SelectObject(mhDC, mhDotPen);
        SetBkMode(mhDC, OPAQUE);
        SetBkColor(mhDC, mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
    }
    else
    {
        SelectObject(mhDC, GetStockObject(DC_PEN));
        SetDCPenColor(mhDC, mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
    }

    MoveToEx(mhDC, x1>>ptGCEIGHTX, y1>>ptGCEIGHTY, NULL);
    LineTo(mhDC, x2>>ptGCEIGHTX, y2>>ptGCEIGHTY);

    if (r2)
        r2 = SetROP2(mhDC, r2);
}

void ptGCGDI::Polygon(const ptCoord* const pPoly, const bbUINT points, const ptPEN pen)
{
    HBRUSH const hbrush = CreateSolidBrush(mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
    HBRUSH const hbrushOld = (HBRUSH) SelectObject(mhDC, hbrush);

    SelectObject(mhDC,GetStockObject(DC_PEN));
    SetDCPenColor(mhDC, mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);

    #if (ptGCEIGHTX==0) && (ptGCEIGHTY==0)
    ::Polygon(mhDC, (const POINT*)pPoly, points);
    #else
    // xxx not supported yet
    #endif

    SelectObject(mhDC, hbrushOld);
    DeleteObject(hbrush);
}

void ptGCGDI::FillCircle(int x, int y, bbUINT rad, const ptPEN pen)
{
    HBRUSH const hbrush = CreateSolidBrush(mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
    HBRUSH const hbrushOld = (HBRUSH) SelectObject(mhDC, hbrush);

    SelectObject(mhDC,GetStockObject(DC_PEN));
    SetDCPenColor(mhDC, mpLogPal->mpRGB[pen & ptPENCOLMASK] & 0xFFFFFFUL);
    
    Ellipse(mhDC, (x-rad)>>ptGCEIGHTX, (y-rad)>>ptGCEIGHTY, (x+rad)>>ptGCEIGHTX, (y+rad)>>ptGCEIGHTY);

    SelectObject(mhDC, hbrushOld);
    DeleteObject(hbrush);
}

bbUINT ptGCGDI::MarkupText(int x, int y, const bbU32* pText, const ptMarkupInfo* const pInfo, bbUINT const linespacing)
{
    x>>=ptGCEIGHTX; 
    y>>=ptGCEIGHTY;
    int const x_org = x;

    ptFont* pFont = pInfo->mpFont[0];
    const bbUINT height = pFont->GetHeight() + linespacing;
    const bbUINT pitch = ((mWidth >> ptGCEIGHTX) + ptFONT_MAXWIDTH*2 + 3) &~ 3;
    if(EnsureLineCache(pitch * height) != bbEOK)
        return 0;

    bbU8* pData = mpLineCache;

    mpBMI->bmiHeader.biWidth = (LONG)pitch;
    mpBMI->bmiHeader.biHeight = -(int)height;
    mpBMI->bmiHeader.biCompression = BI_RGB;
    mpBMI->bmiHeader.biBitCount = 8;
    CachePal(mpLogPal, 0);

    for(;;)
    {
        bbU32 cp = *(pText++);
        bbU32 cp2 = *pText & 0x3FFFFFUL;

        if (cp2 == 0x3FFFFEUL)
            ++pText;

        if (cp == 0xFFFFFFFFUL) // EOL?
        {
            cp = (bbU32)pData - (bbU32)mpLineCache;
            if (cp > pitch)
                cp = pitch;
            SetDIBitsToDevice(mhDC,
                              x, y,
                              cp, height,
                              0, 0,
                              0, height,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            return (x - x_org + (DWORD)pData - (DWORD)mpLineCache) << ptGCEIGHTX;
        }

        bbUINT const fgcol = pInfo->mFGCol[(cp >> ptGCMT_FGPOS) & ptGCMT_FGMASK];
        ptPEN  const bgpen = pInfo->mBGPen[cp >> ptGCMT_BGPOS];
        pFont = pInfo->mpFont[(cp >> ptGCMT_FONTPOS) & ptGCMT_FONTMASK];

        cp &= ptGCMT_MASK;
        if (cp >= 0x110000UL)
            cp = pFont->mUkCP; // outside UNICODE codepage

        const bbU8* pSrc = pFont->GetBankNoReplace(cp);
        bbU32 tmp;

        if (!pSrc || ((tmp = *((bbU32*)pSrc + (cp & (ptFONT_BANKSIZE-1))))==0)) // glyph not existent
        {
            // preserve the widechar property of the unknown character
            #if (bbCPG == bbCPG_UNICODE) || (bbCPG == bbCPG_UCS)
            if (cp2 == 0x3FFFFEUL)
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
        tmp>>=26;

        if (((bbU32)pData-(bbU32)mpLineCache+tmp) <= pitch)
        {
            bbU8* const pDataSave = pData;

            bbUINT yctr = linespacing;
            while (yctr)
            {
                bbU8* pTmp = pData;
                pData += pitch;
                bbUINT width = tmp;
                do
                {
                    *(pTmp++) = bgpen;
                } while (--width);
                --yctr;
            }

            yctr = pFont->GetHeight();
            do
            {
                bbU8* pTmp = pData;
                pData += pitch;

                bbUINT width = tmp;

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

            } while (--yctr > 0);

            pData = pDataSave;
        }

        if ((int)(x + tmp) > 0)
            pData += tmp;
        else
            x += tmp;
    }
}

bbUINT ptGCGDI::Text(int x, int y, const bbCHAR* pMarkup, bbUINT fgcol, ptPEN bgpen, bbUINT const font)
{
    x>>=ptGCEIGHTX; 
    y>>=ptGCEIGHTY;

    ptFont* pFont = ptgFontMan.mFonts[font];

/*
    if (bbEOK != EnsureMaskBmp(mWidth >> ptGCEIGHTX, pFont->GetHeight()))
        return 0;
*/
    const bbUINT pitch = ((mWidth >> ptGCEIGHTX) + 3) &~ 3;
    if(EnsureLineCache(pitch * pFont->GetHeight()) != bbEOK)
        return 0;

    bbU8* pData = mpLineCache;

    mpBMI->bmiHeader.biWidth = (LONG)pitch;
    mpBMI->bmiHeader.biHeight = -(int)pFont->GetHeight();
    mpBMI->bmiHeader.biCompression = BI_RGB;
    mpBMI->bmiHeader.biBitCount = 8;

    CachePal(mpLogPal, 0);

    for(;;)
    {
        bbCHARCP cp;
        bbCP_NEXT_PTR(pMarkup, cp)

        if (cp >= 0x110000UL)
            cp = pFont->mUkCP; // outside UNICODE codepage

        if (cp == 0)
        {
            cp = *(pMarkup++);

            bbASSERT(cp <= 5);

            switch (cp)
            {
            case 1: 
            ptGCGDI_MarkupText_out:
                /*{
                HBRUSH const hbrush = CreateSolidBrush(mpLogPal->mpRGB[fgcol] & 0xFFFFFFUL);
                HBRUSH const hbrushOld = (HBRUSH) SelectObject(mhDC, hbrush);
                cp = MaskBlt(mhDC,
                        x, y,
                        (DWORD)pData - (DWORD)mpLineCache, pFont->GetHeight(),
                        mhDC,
                        0, 0,
                        mhMask,
                        0, 0,
                        MAKEROP4(PATCOPY, SRCPAINT)); // mask 1 -> use fore ROP, mask 0 -> use back ROP
                cp = GetLastError();
                SelectObject(mhDC, hbrushOld);
                DeleteObject(hbrush);
                }*/
                SetDIBitsToDevice(mhDC,
                                  x, y,
                                  (DWORD)pData - (DWORD)mpLineCache, pFont->GetHeight(),
                                  0, 0,
                                  0, pFont->GetHeight(),
                                  mpLineCache,
                                  mpBMI,
                                  DIB_RGB_COLORS);
                return ((DWORD)pData - (DWORD)mpLineCache) << ptGCEIGHTX;

            case 2:
                fgcol = *(pMarkup++);
                continue;
            case 3:
                bgpen = (bgpen & ptPENMASK) | ((bbUINT)*(pMarkup++) & 0xFFU);
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
        tmp>>=26;

        if (((bbU32)pData-(bbU32)mpLineCache+tmp) > pitch)
            goto ptGCGDI_MarkupText_out;
        
        bbUINT yctr = pFont->GetHeight();
        do
        {
            bbU8* pTmp = pData;
            pData += pitch;

            bbUINT width = tmp;

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
                /*
                bbUINT pix = (bbUINT) *pSrc;
                bbU8* pDst = mpMask + (((bbUPTR)pTmp - (bbUPTR)mpLineCache)>>3);
                bbUINT b = (bbUINT)pTmp & 7;
                *pDst = (*pDst &~ (0xFF<<b)) | (pix<<b);
                b=8-b;
                *(pDst+1) = (*(pDst+1)&~ (0xFF>>b)) | (pix>>b);
                */

                bbUINT bits = (bbUINT) *(pSrc++);
                do
                {
                    *(pTmp++) = (bits & 1) ? fgcol : bgpen;
                    bits >>= 1;
                } while (--width);
            }

        } while (--yctr > 0);

        pData -= pitch * pFont->GetHeight() - tmp;
    }
}

void ptGCGDI::CachePal(ptPal* const pPal, bbUINT size)
{
    bbU32 const newhash = (bbU32)pPal ^ pPal->mSyncPt;
    
    if (newhash == mCachedPal)
        return;

    mCachedPal = newhash;

    if (!size)
        size = pPal->mColCount;

    bbASSERT(size <= pPal->mColCount);
    bbASSERT((size-1) < 256);

    const bbU32* const pRGB = pPal->mpRGB;
    do
    {
        bbU32 rgb = pRGB[--size];
        #if bbCPUE==bbCPUE_LE
        *(bbU16*)&mpBMI->bmiColors[size] = (bbU16)rgb; // green
        #else
        mpBMI->bmiColors[size].rgbGreen = (bbU8)(rgb>>8); // green
        #endif
        mpBMI->bmiColors[size].rgbRed   = (bbU8)rgb;
        mpBMI->bmiColors[size].rgbBlue  = (bbU8)(rgb>>16);
    } while(size);
}

bbERR ptGCGDI::EnsureLineCache(bbUINT const linesize)
{
    if (linesize > mLineCacheSize)
    {
        if (bbMemRealloc(linesize, (void**)&mpLineCache) != bbEOK)
            return bbELAST;
        mLineCacheSize = linesize;
    }
    return bbEOK;
}

void ptGCGDI::Sprite(int x, int y, const ptSprite* const pSprite)
{
    bbU8* pData = pSprite->pData;
    bbU8* pLineCache;
    bbU8* pLineCacheEnd;
    const bbS16* pYUV2RGB;
    bbU32 i;
    bbU32 linesize;
    bbU8* pDataU;
    bbU8* pDataV;
    bbU32 Yoffs;
    bbS16 yuv2rgb[12];

    mpBMI->bmiHeader.biWidth = pSprite->width;
    mpBMI->bmiHeader.biHeight = -(int)1;

    x>>=ptGCEIGHTX; 
    y>>=ptGCEIGHTY;
    int const y_end = y + pSprite->height;

    if (ptColFmtIsYUV(pSprite->colfmt))
    {
        pYUV2RGB = pSprite->pYUV2RGB;
        if (ptgColFmtInfo[pSprite->colfmt].flags & ptCOLFMTFLAG_SWAPUV) // VU order?
        {
            for(i=0; i<=9; i+=3) // swap UV coeffs
            {
                yuv2rgb[i+0] = pYUV2RGB[i+0];
                yuv2rgb[i+2] = pYUV2RGB[i+1];
                yuv2rgb[i+1] = pYUV2RGB[i+2];
            }
            pYUV2RGB = yuv2rgb;
        }
    }

    switch (pSprite->colfmt)
    {
    case ptCOLFMT_1BPP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 1;

        CachePal(pSprite->pPal, 2);

        if (pSprite->bitorder == ptBITORDER_LSBLEFT)
        {
            linesize = pSprite->width>>3;
            if (pSprite->width & 7) ++linesize;
            if (EnsureLineCache(linesize) != bbEOK)
                return;

            pLineCache = mpLineCache;
            pLineCacheEnd = pLineCache + linesize;
            while (y < y_end)
            {
                while (pLineCache < pLineCacheEnd)
                {
                    static const bbU8 gBitReverse[16] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };
                    register bbUINT a = *(pData++);
                    a = (bbUINT)gBitReverse[a>>4] | ((bbUINT)gBitReverse[a&0xF]<<4);
                    *(pLineCache++) = a;
                }
            
                pData += pSprite->stride - linesize;
                pLineCache -= linesize;

                SetDIBitsToDevice(mhDC, 
                                  x, y++,
                                  pSprite->width, 1, 
                                  0, 0,
                                  0, 1,
                                  pLineCache,
                                  mpBMI,
                                  DIB_RGB_COLORS);
            }
            return;
        }

        break;
    case ptCOLFMT_2BPP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 4;

        CachePal(pSprite->pPal, 4);

        linesize = (pSprite->width>>1) + (pSprite->width & 1);
        linesize = (linesize + 1) &~ 1UL;
        if (EnsureLineCache(linesize) != bbEOK)
            return;

        pLineCache = mpLineCache;
        while (y < y_end)
        {
            if (pSprite->bitorder == ptBITORDER_LSBLEFT)
            {
                for (i=0; i<linesize; i+=2)
                {
                    register bbUINT a = pData[i>>1]; // get 4 pixels
                    register bbUINT b = (a&0x30)|(a>>6);
                    a&=15; a = ((a<<4) | (a>>2)) & 0x33;
                    pLineCache[i+0] = a;
                    pLineCache[i+1] = b;
                }
            }
            else
            {
                for (i=0; i<linesize; i+=2)
                {
                    register bbUINT a = pData[i>>1]; // get 4 pixels
                    register bbUINT b = ((a<<2)&0x30)|(a&3);
                    a = ((a&0xC0)>>2) | ((a>>4)&3);
                    pLineCache[i+0] = a;
                    pLineCache[i+1] = b;
                }
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              pLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride;
        }
        return;
    case ptCOLFMT_4BPP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 4;
        CachePal(pSprite->pPal, 16);

        if (pSprite->bitorder == ptBITORDER_LSBLEFT)
        {
            linesize = (pSprite->width>>1) + (pSprite->width & 1);
            if (EnsureLineCache(linesize) != bbEOK)
                return;

            pLineCache = mpLineCache;
            pLineCacheEnd = pLineCache + linesize;
            while (y < y_end)
            {
                while (pLineCache < pLineCacheEnd)
                {
                    register bbUINT a = *(pData++);
                    *(pLineCache++) = (a>>4)|(a<<4);
                }
            
                pData += pSprite->stride - linesize;
                pLineCache -= linesize;

                SetDIBitsToDevice(mhDC, 
                                  x, y++,
                                  pSprite->width, 1, 
                                  0, 0,
                                  0, 1,
                                  pLineCache,
                                  mpBMI,
                                  DIB_RGB_COLORS);
            }
            return;
        }

        break;
    case ptCOLFMT_8BPP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 8;
        bbASSERT(pSprite->pPal->mColCount <= 256);
        CachePal(pSprite->pPal, 0);
        break;
    case ptCOLFMT_2BPPP:
    case ptCOLFMT_3BPPP:
    case ptCOLFMT_4BPPP:
    case ptCOLFMT_5BPPP:
    case ptCOLFMT_6BPPP:
    case ptCOLFMT_7BPPP:
    case ptCOLFMT_8BPPP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 8;
        bbASSERT(pSprite->pPal->mColCount <= 256);
        CachePal(pSprite->pPal, 0);

        linesize = pSprite->width;
        linesize = (linesize + 7) &~ 7UL;
        if (EnsureLineCache(linesize) != bbEOK)
            return;

        pLineCacheEnd = mpLineCache + linesize;
        Yoffs = 0;
        linesize = ptgColFmtInfo[pSprite->colfmt].PlaneCount;
        
        while (y < y_end)
        {
            pData = pSprite->pPlane[0] + Yoffs;
            pLineCache = mpLineCache;

            if (pSprite->bitorder == ptBITORDER_LSBLEFT)
            {
                static const bbU32 gBitExtend[16] =
                {
                    0x00000000UL,0x00000001UL,0x00000100UL,0x00000101UL,
                    0x00010000UL,0x00010001UL,0x00010100UL,0x00010101UL,
                    0x01000000UL,0x01000001UL,0x01000100UL,0x01000101UL,
                    0x01010000UL,0x01010001UL,0x01010100UL,0x01010101UL
                };

                while (pLineCache < pLineCacheEnd)
                {
                    register bbUINT c = *(pData++);
                    *(bbU32*)(pLineCache+0) = gBitExtend[c&0xF];
                    *(bbU32*)(pLineCache+4) = gBitExtend[c>>4];
                    pLineCache+=8;
                }

                i = 1;
                do
                {
                    pLineCache = mpLineCache;
                    pData = pSprite->pPlane[i] + Yoffs;
                    while (pLineCache < pLineCacheEnd)
                    {
                        register bbUINT c = *(pData++);
                        *(bbU32*)(pLineCache+0) = *(bbU32*)(pLineCache+0) | (gBitExtend[c&0xF]<<i);
                        *(bbU32*)(pLineCache+4) = *(bbU32*)(pLineCache+4) | (gBitExtend[c>>4]<<i);
                        pLineCache+=8;
                    }
                } while (++i < linesize);
            }
            else
            {
                static const bbU32 gBitExtendInv[16] =
                {
                    0x00000000UL,0x01000000UL,0x00010000UL,0x01010000UL,
                    0x00000100UL,0x01000100UL,0x00010100UL,0x01010100UL,
                    0x00000001UL,0x01000001UL,0x00010001UL,0x01010001UL,
                    0x00000101UL,0x01000101UL,0x00010101UL,0x01010101UL
                };

                while (pLineCache < pLineCacheEnd)
                {
                    register bbUINT c = *(pData++);
                    *(bbU32*)(pLineCache+0) = gBitExtendInv[c>>4];
                    *(bbU32*)(pLineCache+4) = gBitExtendInv[c&0xF];
                    pLineCache+=8;
                }

                i = 1;
                do
                {
                    pLineCache = mpLineCache;
                    pData = pSprite->pPlane[i] + Yoffs;
                    while (pLineCache < pLineCacheEnd)
                    {
                        register bbUINT c = *(pData++);
                        *(bbU32*)(pLineCache+0) = *(bbU32*)(pLineCache+0) | (gBitExtendInv[c>>4]<<i);
                        *(bbU32*)(pLineCache+4) = *(bbU32*)(pLineCache+4) | (gBitExtendInv[c&0xF]<<i);
                        pLineCache+=8;
                    }
                } while (++i < linesize);
            }

            Yoffs += pSprite->stride;

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);
        }
        return;
    case ptCOLFMT_RGB565:
        mpBMI->bmiHeader.biCompression = BI_BITFIELDS;
        mpBMI->bmiHeader.biBitCount = 16;
        *(DWORD*)&mpBMI->bmiColors[0] = 0x001F; //R5
        *(DWORD*)&mpBMI->bmiColors[1] = 0x07E0; //G6
        *(DWORD*)&mpBMI->bmiColors[2] = 0xF800; //B5
        mCachedPal = 0;
        break;
    case ptCOLFMT_RGB888:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;

        if (EnsureLineCache(linesize = pSprite->width*3) != bbEOK)
            return;

        pLineCache = mpLineCache;
        while (y < y_end)
        {
            for (i=0; i<linesize; i+=3)
            {
                pLineCache[i+0] = pData[i+2]; // B
                pLineCache[i+1] = pData[i+1]; // G
                pLineCache[i+2] = pData[i+0]; // R
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              pLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride;
        }
        return;
    case ptCOLFMT_BGR888:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        break;
    case ptCOLFMT_RGBA1555:
        mpBMI->bmiHeader.biCompression = BI_BITFIELDS;
        mpBMI->bmiHeader.biBitCount = 16;
        *(DWORD*)&mpBMI->bmiColors[0] = 0x001F; //R5
        *(DWORD*)&mpBMI->bmiColors[1] = 0x03E0; //G5
        *(DWORD*)&mpBMI->bmiColors[2] = 0x7C00; //B5
        mCachedPal = 0;
        break;
    case ptCOLFMT_RGBA4444:
        mpBMI->bmiHeader.biCompression = BI_BITFIELDS;
        mpBMI->bmiHeader.biBitCount = 16;
        *(DWORD*)&mpBMI->bmiColors[0] = 0x000F; //R4
        *(DWORD*)&mpBMI->bmiColors[1] = 0x00F0; //G4
        *(DWORD*)&mpBMI->bmiColors[2] = 0x0F00; //B4
        mCachedPal = 0;
        break;
    case ptCOLFMT_RGBA8888:
        mpBMI->bmiHeader.biCompression = BI_BITFIELDS;
        mpBMI->bmiHeader.biBitCount = 32;
        *(DWORD*)&mpBMI->bmiColors[0] = 0x000000FF; //R8
        *(DWORD*)&mpBMI->bmiColors[1] = 0x0000FF00; //G8
        *(DWORD*)&mpBMI->bmiColors[2] = 0x00FF0000; //B8
        mCachedPal = 0;
        break;
    case ptCOLFMT_BGRA8888:
        mpBMI->bmiHeader.biCompression = BI_BITFIELDS;
        mpBMI->bmiHeader.biBitCount = 32;
        *(DWORD*)&mpBMI->bmiColors[0] = 0x00FF0000; //R8
        *(DWORD*)&mpBMI->bmiColors[1] = 0x0000FF00; //G8
        *(DWORD*)&mpBMI->bmiColors[2] = 0x000000FF; //B8
        mCachedPal = 0;
        break;
    case ptCOLFMT_YUV420P:
    case ptCOLFMT_YUV420P_YV12:
    case ptCOLFMT_YUV420P_IMC1:
    case ptCOLFMT_YUV420P_IMC3:
    case ptCOLFMT_YUV420P_IMC4:
    case ptCOLFMT_YUV420P_IMC2:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        mpBMI->bmiHeader.biHeight = -(int)2;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much

        i = linesize*3;
        i = (i + 3) &~ 3;
        if (EnsureLineCache(i<<1) != bbEOK)
            return;

        bbASSERT((pSprite->height & 1) == 0);

        Yoffs = 0;
        pDataU = pSprite->pPlane[2];
        pDataV = pSprite->pPlane[3];

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            pData = pSprite->pPlane[0] + Yoffs;
            i=2;
            do
            {
                pLineCacheEnd = pLineCache + linesize*3;
                while (pLineCache < pLineCacheEnd)
                {
                    int const y0 = ((int)*(pData++)  + (int)pYUV2RGB[0]);
                    int const y1 = ((int)*(pData++)  + (int)pYUV2RGB[0]);
                    int const u  = ((int)*(pDataU++) + (int)pYUV2RGB[1]);
                    int const v  = ((int)*(pDataV++) + (int)pYUV2RGB[2]);

                    int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                    register int p;
                    if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[0] = p; // B0
                    if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[3] = p; // B1
                    tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                    if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[1] = p; // G0
                    if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[4] = p; // G1
                    tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                    if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[2] = p; // R0
                    if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[5] = p; // R1
                    pLineCache += 6;
                }

                pLineCache = (bbU8*)(((bbUPTR)pLineCache-(pSprite->width & 1)*3+3) &~ (bbUPTR)3);
                pData = pSprite->pPlane[1] + Yoffs;
                pDataU -= linesize>>1;
                pDataV -= linesize>>1;

            } while(--i);

            SetDIBitsToDevice(mhDC, 
                              x, y,
                              pSprite->width, 2,
                              0, 0,
                              0, 2,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            Yoffs  += pSprite->stride<<1;
            pDataU += pSprite->strideUV;
            pDataV += pSprite->strideUV;
            y+=2;
        }
        return;
    case ptCOLFMT_YUV420P_NV12:
    case ptCOLFMT_YUV420P_NV21:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        mpBMI->bmiHeader.biHeight = -(int)2;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much

        i = linesize*3;
        i = (i + 3) &~ 3;
        if (EnsureLineCache(i<<1) != bbEOK)
            return;

        bbASSERT((pSprite->height & 1) == 0);

        Yoffs = 0;
        pDataU = pSprite->pPlane[2];

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            pData = pSprite->pPlane[0] + Yoffs;
            i=2;
            do
            {
                pData = pSprite->pPlane[i&1] + Yoffs;

                pLineCacheEnd = pLineCache + linesize*3;
                while (pLineCache < pLineCacheEnd)
                {
                    int const y0 = ((int)*(pData++)  + (int)pYUV2RGB[0]);
                    int const y1 = ((int)*(pData++)  + (int)pYUV2RGB[0]);
                    int const u  = ((int)*(pDataU++) + (int)pYUV2RGB[1]);
                    int const v  = ((int)*(pDataU++) + (int)pYUV2RGB[2]);

                    int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                    register int p;
                    if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[0] = p; // B0
                    if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[3] = p; // B1
                    tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                    if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[1] = p; // G0
                    if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[4] = p; // G1
                    tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                    if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[2] = p; // R0
                    if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>=256) p=255;
                    pLineCache[5] = p; // R1
                    pLineCache += 6;
                }

                pLineCache = (bbU8*)(((bbUPTR)pLineCache-(pSprite->width & 1)*3+3) &~ (bbUPTR)3);
                pData = pSprite->pPlane[1] + Yoffs;
                pDataU -= linesize;

            } while(--i);

            SetDIBitsToDevice(mhDC, 
                              x, y,
                              pSprite->width, 2,
                              0, 0,
                              0, 2,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            Yoffs  += pSprite->stride<<1;
            pDataU += pSprite->strideUV;
            y+=2;
        }
        return;
    case ptCOLFMT_YUV420P_12:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        mpBMI->bmiHeader.biHeight = -(int)2;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much

        i = linesize*3;
        i = (i + 3) &~ 3;
        if (EnsureLineCache(i<<1) != bbEOK)
            return;

        bbASSERT((pSprite->height & 1) == 0);

        Yoffs = 0;
        pDataU = pSprite->pPlane[2];
        pDataV = pSprite->pPlane[3];

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            pData = pSprite->pPlane[0] + Yoffs;
            i=2;
            do
            {
                pLineCacheEnd = pLineCache + linesize*3;

                if (pSprite->endian == ptENDIAN_LE)
                {
                    while (pLineCache < pLineCacheEnd)
                    {
                        int const y0 = ((int)(bbLD16LE(pData)>>4) + (int)pYUV2RGB[0]); pData+=2;
                        int const y1 = ((int)(bbLD16LE(pData)>>4) + (int)pYUV2RGB[0]); pData+=2;
                        int const u  = ((int)(bbLD16LE(pDataU)>>4) + (int)pYUV2RGB[1]); pDataU+=2;
                        int const v  = ((int)(bbLD16LE(pDataV)>>4) + (int)pYUV2RGB[2]); pDataV+=2;

                        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                        register int p;
                        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[0] = p; // B0
                        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[3] = p; // B1
                        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[1] = p; // G0
                        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[4] = p; // G1
                        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[2] = p; // R0
                        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[5] = p; // R1
                        pLineCache += 6;
                    }
                }
                else
                {
                    while (pLineCache < pLineCacheEnd)
                    {
                        int const y0 = ((int)(bbLD16BE(pData)>>4) + (int)pYUV2RGB[0]); pData+=2;
                        int const y1 = ((int)(bbLD16BE(pData)>>4) + (int)pYUV2RGB[0]); pData+=2;
                        int const u  = ((int)(bbLD16BE(pDataU)>>4) + (int)pYUV2RGB[1]); pDataU+=2;
                        int const v  = ((int)(bbLD16BE(pDataV)>>4) + (int)pYUV2RGB[2]); pDataV+=2;

                        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                        register int p;
                        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[0] = p; // B0
                        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[3] = p; // B1
                        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[1] = p; // G0
                        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[4] = p; // G1
                        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[2] = p; // R0
                        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                        if (p>255) p=255;
                        pLineCache[5] = p; // R1
                        pLineCache += 6;
                    }
                }

                pLineCache = (bbU8*)(((bbUPTR)pLineCache-(pSprite->width & 1)*3+3) &~ (bbUPTR)3);
                pData = pSprite->pPlane[1] + Yoffs;
                pDataU -= linesize;
                pDataV -= linesize;

            } while(--i);

            SetDIBitsToDevice(mhDC, 
                              x, y,
                              pSprite->width, 2,
                              0, 0,
                              0, 2,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            Yoffs  += pSprite->stride<<1;
            pDataU += pSprite->strideUV;
            pDataV += pSprite->strideUV;
            y+=2;
        }
        return;
    case ptCOLFMT_YUV420P_16:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        mpBMI->bmiHeader.biHeight = -(int)2;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much

        i = linesize*3;
        i = (i + 3) &~ 3;
        if (EnsureLineCache(i<<1) != bbEOK)
            return;

        bbASSERT((pSprite->height & 1) == 0);

        Yoffs = 0;
        pDataU = pSprite->pPlane[2];
        pDataV = pSprite->pPlane[3];

        if (pSprite->endian == ptENDIAN_LE)
            Yoffs++, pDataU++, pDataV++;

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            pData = pSprite->pPlane[0] + Yoffs;
            i=2;
            do
            {
                pLineCacheEnd = pLineCache + linesize*3;

                while (pLineCache < pLineCacheEnd)
                {
                    int const y0 = ((int)*pData + (int)pYUV2RGB[0]); pData+=2;
                    int const y1 = ((int)*pData + (int)pYUV2RGB[0]); pData+=2;
                    int const u  = ((int)*pDataU + (int)pYUV2RGB[1]); pDataU+=2;
                    int const v  = ((int)*pDataV + (int)pYUV2RGB[2]); pDataV+=2;

                    int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                    register int p;
                    if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[0] = p; // B0
                    if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[3] = p; // B1
                    tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                    if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[1] = p; // G0
                    if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[4] = p; // G1
                    tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                    if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[2] = p; // R0
                    if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[5] = p; // R1
                    pLineCache += 6;
                }

                pLineCache = (bbU8*)(((bbUPTR)pLineCache-(pSprite->width & 1)*3+3) &~ (bbUPTR)3);
                pData = pSprite->pPlane[1] + Yoffs;
                pDataU -= linesize;
                pDataV -= linesize;

            } while(--i);

            SetDIBitsToDevice(mhDC, 
                              x, y,
                              pSprite->width, 2,
                              0, 0,
                              0, 2,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            Yoffs  += pSprite->stride<<1;
            pDataU += pSprite->strideUV;
            pDataV += pSprite->strideUV;
            y+=2;
        }
        return;
    case ptCOLFMT_YUYV:
    case ptCOLFMT_YVYU:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much
        if (EnsureLineCache(linesize*3) != bbEOK)
            return;

        linesize >>= 1;
        pLineCacheEnd = mpLineCache + linesize*6;
        while (y < y_end)
        {
            pLineCache = mpLineCache;
            while (pLineCache < pLineCacheEnd)
            {
                int const y0 = ((int)pData[0] + pYUV2RGB[0]);
                int const u  = ((int)pData[1] + pYUV2RGB[1]);
                int const y1 = ((int)pData[2] + pYUV2RGB[0]);
                int const v  = ((int)pData[3] + pYUV2RGB[2]);
                pData += 4;
                int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                register int p;
                if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[0] = p; // B0
                if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[3] = p; // B1
                tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[1] = p; // G0
                if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[4] = p; // G1
                tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[2] = p; // R0
                if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[5] = p; // R1
                pLineCache += 6;
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride - (linesize << 2);
        }
        return;
    case ptCOLFMT_UYVY:
    case ptCOLFMT_VYUY:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much
        if (EnsureLineCache(linesize*3) != bbEOK)
            return;

        linesize >>= 1;
        pLineCacheEnd = mpLineCache + linesize*6;
        while (y < y_end)
        {
            pLineCache = mpLineCache;
            while (pLineCache < pLineCacheEnd)
            {
                int const u  = ((int)pData[0] + pYUV2RGB[1]);
                int const y0 = ((int)pData[1] + pYUV2RGB[0]);
                int const v  = ((int)pData[2] + pYUV2RGB[2]);
                int const y1 = ((int)pData[3] + pYUV2RGB[0]);
                pData += 4;
                int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                register int p;
                if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[0] = p; // B0
                if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[3] = p; // B1
                tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[1] = p; // G0
                if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[4] = p; // G1
                tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[2] = p; // R0
                if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[5] = p; // R1
                pLineCache += 6;
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride - (linesize << 2);
        }
        return;
    case ptCOLFMT_YUV422P:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much
        if (EnsureLineCache(linesize*3) != bbEOK)
            return;

        pDataU   = pSprite->pPlane[1];
        pDataV   = pSprite->pPlane[2];
        pLineCacheEnd = mpLineCache + linesize*3;
        Yoffs    = pSprite->strideUV - (linesize>>1);

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            while (pLineCache < pLineCacheEnd)
            {
                int const y0 = ((int)*(pData++)  + pYUV2RGB[0]);
                int const y1 = ((int)*(pData++)  + pYUV2RGB[0]);
                int const u  = ((int)*(pDataU++) + pYUV2RGB[1]);
                int const v  = ((int)*(pDataV++) + pYUV2RGB[2]);

                int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                register int p;
                if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[0] = p; // B0
                if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[3] = p; // B1
                tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[1] = p; // G0
                if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[4] = p; // G1
                tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[2] = p; // R0
                if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[5] = p; // R1
                pLineCache += 6;
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData  += pSprite->stride - linesize;
            pDataU += Yoffs;
            pDataV += Yoffs;
        }
        return;
    case ptCOLFMT_YUV422RP:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        mpBMI->bmiHeader.biHeight = -(int)2;
        linesize = pSprite->width + (pSprite->width & 1); // for odd width we draw one pixel too much

        i = linesize*3;
        i = (i + 3) &~ 3;
        if (EnsureLineCache(i<<1) != bbEOK)
            return;

        bbASSERT((pSprite->height & 1) == 0);

        Yoffs = 0;
        pDataU = pSprite->pPlane[2];
        pDataV = pSprite->pPlane[3];

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            pData = pSprite->pPlane[0] + Yoffs;
            i=2;
            do
            {
                pLineCacheEnd = pLineCache + linesize*3;
                while (pLineCache < pLineCacheEnd)
                {
                    int const y = ((int)*(pData++)  + pYUV2RGB[0]);
                    int const u = ((int)*(pDataU++) + pYUV2RGB[1]);
                    int const v = ((int)*(pDataV++) + pYUV2RGB[2]);

                    register int p;
                    if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[0] = p; // B
                    if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[1] = p; // G
                    if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
                    if (p>255) p=255;
                    pLineCache[2] = p; // R
                    pLineCache+=3;

                }

                pLineCache = (bbU8*)(((bbUPTR)pLineCache-(pSprite->width & 1)*3+3) &~ (bbUPTR)3);
                pData = pSprite->pPlane[1] + Yoffs;
                pDataU -= linesize;
                pDataV -= linesize;

            } while(--i);

            SetDIBitsToDevice(mhDC, 
                              x, y,
                              pSprite->width, 2,
                              0, 0,
                              0, 2,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            Yoffs  += pSprite->stride<<1;
            pDataU += pSprite->strideUV;
            pDataV += pSprite->strideUV;
            y+=2;
        }
        return;
    case ptCOLFMT_YUV444:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        if (EnsureLineCache(linesize = pSprite->width*3) != bbEOK)
            return;

        pLineCache = mpLineCache;
        while (y < y_end)
        {
            for (i=0; i<linesize; i+=3)
            {
                int const y = ((int)pData[i+0] + pYUV2RGB[0]);
                int const u = ((int)pData[i+1] + pYUV2RGB[1]);
                int const v = ((int)pData[i+2] + pYUV2RGB[2]);
                register int p;
                if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[i+0] = p; // B
                if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[i+1] = p; // G
                if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[i+2] = p; // R
            }

            SetDIBitsToDevice(mhDC,
                              x, y++,
                              pSprite->width, 1,
                              0, 0,
                              0, 1,
                              pLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride;
        }
        return;

    case ptCOLFMT_AYUV:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;

        if (EnsureLineCache(linesize = pSprite->width*3) != bbEOK)
            return;
        pLineCache = mpLineCache;
        pLineCacheEnd = pLineCache + linesize;
        while (y < y_end)
        {
            while (pLineCache < pLineCacheEnd)
            {
                int const y = ((int)pData[0] + pYUV2RGB[0]);
                int const u = ((int)pData[1] + pYUV2RGB[1]);
                int const v = ((int)pData[2] + pYUV2RGB[2]); pData+=4;
                register int p; 
                if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[0] = p; // B
                if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[1] = p; // G
                if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[2] = p; // R
                pLineCache += 3;
            }

            pLineCache -= linesize;
            pData += pSprite->stride - (pSprite->width<<2);

            SetDIBitsToDevice(mhDC,
                              x, y++,
                              pSprite->width, 1,
                              0, 0,
                              0, 1,
                              pLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);
        }
        return;
    case ptCOLFMT_YUV444P:
        mpBMI->bmiHeader.biCompression = BI_RGB;
        mpBMI->bmiHeader.biBitCount = 24;
        if (EnsureLineCache(linesize = pSprite->width*3) != bbEOK)
            return;

        pDataU   = pSprite->pPlane[1];
        pDataV   = pSprite->pPlane[2];
        pLineCacheEnd = mpLineCache + linesize;
        i     = pSprite->width;
        Yoffs = pSprite->stride - i;
        i     = pSprite->strideUV - i;

        while (y < y_end)
        {
            pLineCache = mpLineCache;
            while (pLineCache < pLineCacheEnd)
            {
                int const y = ((int)*(pData++)  + pYUV2RGB[0]);
                int const u = ((int)*(pDataU++) + pYUV2RGB[1]);
                int const v = ((int)*(pDataV++) + pYUV2RGB[2]);

                register int p;
                if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[0] = p; // B
                if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[1] = p; // G
                if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
                if (p>=256) p=255;
                pLineCache[2] = p; // R
                pLineCache+=3;
            }

            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              mpLineCache,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData  += Yoffs;
            pDataU += i;
            pDataV += i;
        }
        return;
    default:
        return;
    }

    if (((((pSprite->width * mpBMI->bmiHeader.biBitCount)>>3) + 3) &~ 3) == pSprite->stride)
    {
        y = y_end-y;
        mpBMI->bmiHeader.biHeight = -y;

        SetDIBitsToDevice(mhDC, 
                          x, y,
                          pSprite->width, y, 
                          0, 0,
                          0, y,
                          pData,
                          mpBMI,
                          DIB_RGB_COLORS);
    }
    else
    {
        while (y < y_end)
        {
            SetDIBitsToDevice(mhDC, 
                              x, y++,
                              pSprite->width, 1, 
                              0, 0,
                              0, 1,
                              pData,
                              mpBMI,
                              DIB_RGB_COLORS);

            pData += pSprite->stride;
        }
    }
}

#endif
