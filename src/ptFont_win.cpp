#define _WIN32_WINNT 0x0500

#include "ptFont_win.h"

#if ptUSE_FONTWIN == 1

#include "babel/str.h"

ptFont_win::ptFont_win()
{
    mInfo.mType  = ptFONTTYPE_WIN;
    mhDC   = NULL;
    mhFont = NULL;
    mhBmp  = NULL;
}

ptFont_win::~ptFont_win()
{
    ptFont_win::Clear();
}

void ptFont_win::Clear()
{
    if (mhFont)
    {
        DeleteObject(mhFont);
        mhFont = NULL;
    }

    if (mhBmp)
    {
         DeleteObject(mhBmp);
         mhBmp = NULL;
    }

    if (mhDC)
    {
        DeleteDC(mhDC);
        mhDC = NULL;
    }

    ptFont::Clear(); // frees mem for pages and banks
    mInfo.mType  = ptFONTTYPE_WIN;
}

bbERR ptFont_win::Init(ptFONTFORMAT const format, const bbCHAR* const pName, bbUINT const width, bbUINT const height, ptFONTSTYLE const style, bbUINT const charset, bbU32 const opt)
{
	int        i;
    bbCHARCP   firstcp;
    LOGFONT    lf;
    TEXTMETRIC tm;

    bbMemClear(&lf, sizeof(LOGFONT));
    lf.lfHeight         = height;
    lf.lfWidth          = (opt & ptFONT_WIN_INIT_NOHSCALE) ? 0 : width;
    if (style & ptFONTSTYLE_BOLD) lf.lfWeight = FW_BOLD;
    lf.lfItalic         = (BYTE)(style & ptFONTSTYLE_ITALIC); // 0 or 1
    lf.lfCharSet        = (BYTE)charset; // charset ID in top byte
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    lf.lfOutPrecision   = OUT_RASTER_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = NONANTIALIASED_QUALITY;

    for (i=0; i<LF_FACESIZE-1; i++)
    {
        if ((lf.lfFaceName[i] = pName[i]) == 0) //xxx
            break;
    }
    lf.lfFaceName[i] = 0;
/*   
fh=fopen("c:\\xxx","ab");
fwprintf(fh, TEXT("%s %d %d\n"), pName, width, height);
*/
    Clear();

    if ((width > (ptFONT_MAXWIDTH/2)) || (height > ptFONT_MAXHEIGHT))
        return bbErrSet(bbEBADPARAM);

    // create DC and offscreen bitmap
    if ((mhDC = CreateCompatibleDC(NULL)) == NULL) // also creates 1x1 1bpp bmp object
        goto ptFont_win_Init_err;

    if ((mhBmp = CreateCompatibleBitmap(mhDC, width<<1, height)) == NULL) // mhDC is bound to 1bpp, so we will get a 1bpp bmp
        goto ptFont_win_Init_err;

    // - assert that bitmap is monochrome and of right size
    #ifdef bbDEBUG
    {
    BITMAPCOREHEADER coreheader;
    bbMemClear(&coreheader, sizeof(coreheader));
    coreheader.bcSize = sizeof(BITMAPCOREHEADER);

    bbASSERT(GetDIBits(mhDC, mhBmp, 0, height, NULL, (BITMAPINFO*)&coreheader, DIB_RGB_COLORS) &&
             (coreheader.bcWidth    == (width<<1)) &&
             (coreheader.bcHeight   == height) &&
             (coreheader.bcBitCount == 1) &&
             (coreheader.bcPlanes   == 1));
    }
    #endif

    if ((SelectObject(mhDC, mhBmp)) == NULL)
        goto ptFont_win_Init_err;

    // create windows font

    if ((mhFont = CreateFontIndirect(&lf)) == NULL)
        goto ptFont_win_Init_err;

    if (!GetTextMetrics(mhDC, &tm))
        goto ptFont_win_Init_err;

    if (SelectObject(mhDC, mhFont) == NULL)
        goto ptFont_win_Init_err;

    if ((SetBkColor(mhDC, 0) == CLR_INVALID) || 
        (SetTextColor(mhDC, 0xFFFFFF) == CLR_INVALID))
        goto ptFont_win_Init_err;

    //SetTextAlign(mhDC, TA_TOP|TA_LEFT|TA_NOUPDATECP); is default anyway

    // init font properties
    mInfo.mFormat   = (bbU8)format;
    mInfo.mWidth    = (bbU8)width;
    mInfo.mHeight   = (bbU8)height;
    mInfo.mCodepage = bbCPG_UCS; // Windows does not support bbCPG_UNICODE Unicode 4.0

    // get available unicode pages
    if (GetAvailableCP(NULL, &firstcp) != bbEOK) // also initialized mPages[]
        goto ptFont_win_Init_err0;

    if (SetUnknownCP('.') != bbEOK)
    {
        if (SetUnknownCP(firstcp) != bbEOK)
            goto ptFont_win_Init_err0;
    }

    if (SetUnknownCPW(0x25A1) != bbEOK)
    {
        bbU8* pCPWBmp = EditCP(0x25A1, GetWidth());
        if (pCPWBmp == NULL)
            goto ptFont_win_Init_err0;

        //xxx copy CPW bitmap here, return for now

        if (SetUnknownCPW(0x25A1) != bbEOK)
            goto ptFont_win_Init_err0;
    }

    // set font name
    if ((mInfo.mpFont = bbStrDup(pName)) == NULL)
        goto ptFont_win_Init_err0;
    mInfo.mpName = mInfo.mpFont;

    return bbEOK;

    ptFont_win_Init_err:
    bbErrSet(ptEFONTWININIT);
    ptFont_win_Init_err0:
    Clear();
    return bbELAST;
}

bbERR ptFont_win::GetAvailableCP(const bbU8** pPage, bbCHARCP* const pFirstCP)
{
    int         i;
    DWORD       tmp;
    GLYPHSET*   pGS = NULL;
    bbU32       firstcp, endcp;

    if (pPage)
    {
        firstcp = (bbU32)pFirstCP << 16;
        endcp = firstcp + 0x10000UL;
    }
    else
    {
        firstcp = 0;
        #if (bbSIZEOF_CHARCP > 2)
        endcp = 0x110000UL;
        #else
        endcp = 0x10000UL;
        #endif
    }

    /* activate later for win98
    int charset;
    CHARSETINFO charsetinfo;

    if ((charset = GetTextCharsetInfo(mhDC, &charsetinfo.fs, 0)) == DEFAULT_CHARSET)
    {
        bbASSERT(0); // try to find a font that returns DEFAULT_CHARSET, and investigate later
        goto ptFont_win_Init_err;
    }

    tmp=0; for (i=sizeof(FONTSIGNATURE)/4-1; i>=0; i--) tmp|=((DWORD*)&charsetinfo.fs)[i];
    if (tmp == 0) // No TT-font
    {
        TranslateCharsetInfo((DWORD*)charset, &charsetinfo, TCI_SRCCHARSET);
        // xxx, unicode bitmask will be 0, add codepage tranlation later
    }
    {DWORD info = GetFontLanguageInfo(mhDC);}
    */

    if ((tmp = GetFontUnicodeRanges(mhDC, NULL)) == 0)
        goto ptFont_win_GetAvailableBanks_err;

    if ((pGS = (GLYPHSET*)bbMemAlloc(tmp)) == NULL)
    {
        bbErrSet(bbENOMEM);
        goto ptFont_win_GetAvailableBanks_err0;
    }

    if (GetFontUnicodeRanges(mhDC, pGS) == 0)
        goto ptFont_win_GetAvailableBanks_err;

    bbASSERT(pGS->cRanges);
    if (pPage==NULL) *pFirstCP = pGS->ranges[0].wcLow; // should be save, even if pGS->cRanges is 0

    // - translate available unicode codepoints to either mpPages[], or pPage[] index of available banks
    for (i=0; i<(int)pGS->cRanges; i++)
    {
        bbU32       cp     = (bbU32) pGS->ranges[i].wcLow;
        bbU32 const cp_end = cp + (bbU32) pGS->ranges[i].cGlyphs;

        bbASSERT((i==0) || (pGS->ranges[i].wcLow > pGS->ranges[i-1].wcLow)); // ensure sorting

        do
        {
            // - prevent overflow
            if (cp >= endcp) break; 

            if (pPage)
            {
                // - mark bank as existent
                int const bank = (int)(cp-firstcp) >> ptFONT_BANKSIZE_LOG2;
                if (bank >= 0) pPage[bank] = (const bbU8*)1;

                // - advance to next bank
                cp |= ptFONT_BANKSIZE-1;
            }
            else
            {
                // - mark page as existent
                mpPages[cp>>16] = (const bbU8* const *)1; // Unicode page is existent

                // - advance to next page
                cp |= 0xFFFF;
            }
        } while (++cp < cp_end);
    }

    bbMemFree(pGS);

    return bbEOK;

    ptFont_win_GetAvailableBanks_err:
    bbErrSet(ptEFONTWININIT);
    ptFont_win_GetAvailableBanks_err0:
    if (pGS) bbMemFree(pGS);
    return bbELAST;
}

const bbU8* const* ptFont_win::LoadPage(bbCHARCP const cp)
{
    const bbU8** pPage = NULL;

    bbASSERT(((cp>>16) < 17) && (mpPages[cp>>16] == (const bbU8* const*)1));

    // allocate memory for page
    if ((pPage = (const bbU8**) bbMemAlloc(ptFONT_BANKCOUNT * sizeof(bbU8*))) == NULL)
        goto ptFont_win_LoadPage_err;

    bbMemClear((void*)pPage, ptFONT_BANKCOUNT * sizeof(bbU8*));

    if (GetAvailableCP(pPage, (bbCHARCP*)((bbU32)cp>>16)) != bbEOK)
        goto ptFont_win_LoadPage_err;

    mpPages[cp>>16] = pPage;
    return pPage;

    ptFont_win_LoadPage_err:
    if (pPage) bbMemFree(pPage);
    return NULL;
}

const bbU8* ptFont_win::LoadBank(bbCHARCP const cp)
{
    GLYPHSET* pGS = NULL;
    DWORD*    pBmpBuf = NULL;
    bbU32     firstcp, endcp, data_wr;
    bbUINT    i, chwidth, chwidth8, chwidth32;

    ABC   realwidth;
    TCHAR text[2];
    RECT  rect;

    struct
    {
        BITMAPINFO  bmpinfo;
        RGBQUAD     col1;
    } s;

    // find pointer to bank
    bbASSERT(((cp>>16) < 17) && (mpPages[cp>>16] >= (const bbU8**)4));
    bbU8** const ppBank = (bbU8**)(mpPages[cp>>16] + ((cp >> ptFONT_BANKSIZE_LOG2) & (ptFONT_BANKCOUNT-1)));
    bbASSERT(*ppBank == (bbU8*)1);

    // allocate memory for bank
    chwidth   = GetWidth() << 1;
    chwidth32 = ((chwidth + 31) &~ 31) >> 3;
    chwidth8  = ((chwidth + 7) &~ 7) >> 3;

    bbUINT const bankmemsize = ptFONT_BANKSIZE * ((GetHeight() * chwidth8) + sizeof(bbU32));
    bbU8* const pBank = (bbU8*) bbMemAlloc(bankmemsize);
    if (pBank == NULL)
        goto ptFont_win_LoadBank_err;

    data_wr = (bbU32) ptFONT_BANKSIZE * sizeof(bbU32);
    bbMemClear(pBank, data_wr);

    // get available codepoints from windows font
    if ((firstcp = GetFontUnicodeRanges(mhDC, NULL)) == 0)
        goto ptFont_win_LoadBank_err0;

    if ((pGS = (GLYPHSET*)bbMemAlloc(firstcp)) == NULL)
    {
        bbErrSet(bbENOMEM);
        goto ptFont_win_LoadBank_err;
    }

    if (GetFontUnicodeRanges(mhDC, pGS) == 0)
        goto ptFont_win_LoadBank_err0;

    // copy bitmaps
    firstcp = (bbU32)cp &~ (ptFONT_BANKSIZE-1);
    endcp = firstcp + ptFONT_BANKSIZE;
    text[1] = 0; // 0-terminator

    bbMemClear(&s.bmpinfo, sizeof(BITMAPINFO));
    s.bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    s.bmpinfo.bmiHeader.biWidth = (int)chwidth;
    s.bmpinfo.bmiHeader.biHeight = -(int)GetHeight();
    s.bmpinfo.bmiHeader.biPlanes = s.bmpinfo.bmiHeader.biBitCount = 1;
    s.bmpinfo.bmiHeader.biCompression = BI_RGB;
    s.bmpinfo.bmiHeader.biSizeImage = 0;

    // - alloc char bitmap buffer for DIB
    if ((pBmpBuf = (DWORD*)bbMemAlloc(GetHeight() * chwidth32)) == NULL)
    {
        goto ptFont_win_LoadBank_err;
    }

    rect.left   = rect.top = 0;
    rect.bottom = GetHeight();

    for (i=0; i<pGS->cRanges; i++)
    {
        bbU32 cp_cur = (bbU32) pGS->ranges[i].wcLow;
        bbU32 cp_end = cp_cur + (bbU32) pGS->ranges[i].cGlyphs;

        if (cp_end <= firstcp) continue;
        if (cp_cur < firstcp) cp_cur = firstcp;
        if (cp_cur >= endcp) break;
        if (cp_end > endcp) cp_end = endcp;

        do
        {
            text[0] = (TCHAR)cp_cur;
            bbASSERT(text[1] == 0);

            chwidth = GetWidth();

            if (bbCpgUnicode_IsWide(cp_cur))
                chwidth<<=1;

            realwidth.abcB = chwidth; // default value for failure of GetChar*Width()
            if (mInfo.mFormat != ptFONTFORMAT_WINTRUETYPE)
            {
                GetCharWidth32(mhDC, cp_cur, cp_cur, (int*)&realwidth.abcB);
            }
            else
            {
                GetCharABCWidths(mhDC, cp_cur, cp_cur, &realwidth);
/*
bbUINT j = realwidth.abcB + realwidth.abcA + realwidth.abcC;
if (chwidth != j) fprintf(fh, "%04X %d %d (%d,%d,%d)\n", cp_cur, chwidth, j, realwidth.abcA, realwidth.abcB, realwidth.abcC);
*/
                realwidth.abcB += realwidth.abcA + realwidth.abcC;
            }

            if (chwidth >= realwidth.abcB)
                rect.right = chwidth;
            else
                rect.right = chwidth<<1;

            if (ExtTextOut(mhDC, 0, 0, ETO_NUMERICSLATIN | ETO_OPAQUE, &rect, text, 1, NULL)) // returns 0 on error
            {
                // according to MSDN GetDIBits() must not be called with font still selected in DC,
                // but it seems to work, so let mhFont selected always in mhDC
                GetDIBits(mhDC, mhBmp, 0, GetHeight(), pBmpBuf, &s.bmpinfo, DIB_RGB_COLORS); // ignore error (returns 0 on error)

                *((bbU32*)pBank + (cp_cur - firstcp)) = data_wr | ((bbU32)chwidth << 26);
            
                bbU8* pSrc = (bbU8*)pBmpBuf;
                bbU8* pDst = pBank + data_wr;
                bbUINT y = GetHeight();
                chwidth8 = ((chwidth + 7) &~ 7) >> 3;

                if (realwidth.abcB <= chwidth)
                {
                    do // normal 1:1 copy
                    {
                        int x = (int)chwidth8-1;
                        do
                        { 
                            register bbUINT c = pSrc[x];
                            c = ((c&0xAA)>>1) | ((c&0x55)<<1);
                            c = ((c&0xCC)>>2) | ((c&0x33)<<2);
                            c = (c<<4) | (c>>4);                        
                            pDst[x] = c;
                        } while (--x >= 0);
                        pDst += chwidth8;
                        pSrc += chwidth32;
                    } while (--y);
                }
                else
                {
                    do // copy with 1/2 horizontal scaling
                    {
                        int x = (int)chwidth8-1;
                        do
                        { 
                            register bbUINT d = pSrc[x*2];
                            register bbUINT c = 0;
                            if (d & 0xC0) c=1;
                            if (d & 0x30) c|=2;
                            if (d & 0x0C) c|=4;
                            if (d & 0x03) c|=8;
                            d = pSrc[x*2+1];
                            register bbUINT b = 0;
                            if (d & 0xC0) b=1<<4;
                            if (d & 0x30) b|=2<<4;
                            if (d & 0x0C) b|=4<<4;
                            if (d & 0x03) b|=8<<4;                       
                            pDst[x] = c|b;
                        } while (--x >= 0);
                        pDst += chwidth8;
                        pSrc += chwidth32;
                    } while (--y);
                }

                data_wr += GetHeight() * chwidth8;
            }

        } while (++cp_cur < cp_end);
    }

    bbMemFree(pBmpBuf);
    bbMemFree(pGS);

    *ppBank = pBank;

    // reallocate heap block for bank to size actually written
    if (data_wr < bankmemsize)
    {
        #ifdef bbDEBUG
        i=
        #endif
        bbMemRealloc(data_wr, (void**)ppBank);
        bbASSERT(i == bbEOK);
    }

    return (const bbU8*) *ppBank;

    ptFont_win_LoadBank_err0:
    bbErrSet(ptEFONTWININIT);
    ptFont_win_LoadBank_err:
    if (pBmpBuf) bbMemFree(pBmpBuf);
    if (pGS) bbMemFree(pGS);
    if (pBank) bbMemFree(pBank);

    return NULL;
}

#endif /* ptUSE_FONTWIN */


