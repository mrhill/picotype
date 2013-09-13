#include "babel/mem.h"
#include "babel/str.h"

#include "ptFont.h"

void ptFontInfo::Clear()
{
    if (mpFont != mpName)
        bbMemFree(mpName);
    bbMemFree(mpFont);
    bbMemClear(this, sizeof(ptFontInfo));
}

bbERR ptFontInfo::CopyTo(ptFontInfo* const pTo) const
{
    pTo->Destroy();

    *pTo = *this;
    pTo->mpName = pTo->mpFont = NULL;

    if (mType != ptFONTTYPE_NONE)
    {
        if ((pTo->mpName = pTo->mpFont = bbStrDup(mpFont)) == NULL)
            return bbELAST;

        if (mpFont != mpName)
        {
            if ((pTo->mpName = bbStrDup(mpName)) == NULL)
            {
                bbMemFreeNull((void**)&pTo->mpFont);
                return bbELAST;
            }
        }
    }
    return bbEOK;
}

bbCHAR* ptFontInfo::ToStr()
{
    bbUINT nameLen = mpFont ? bbStrLen(mpFont) : 0;
    bbCHAR* const pStr = bbStrAlloc(nameLen + 3*8);
    if (pStr)
    {
        switch (mType)
        {
        case ptFONTTYPE_MEM:
            bbSprintf(pStr, bbT("%u,%s"), mType, mpFont);
            break;
        case ptFONTTYPE_WIN:
            bbSprintf(pStr, bbT("%u,%s,%u,%u,%u,%u,%u"), mType, mpFont, mFormat, mWidth, mHeight, mStyle, mCharset);
            break;
        default:
            bbSprintf(pStr, bbT("%u"), mType);
            break;
        }
    }
    return pStr;
}

bbERR ptFontInfo::FromStr(const bbCHAR* pStr)
{
    bbERR err = bbELAST;
    bbUINT count;
    bbU32 type;

    if (!pStr)
    {
        Clear();
        mType = ptFONTTYPE_DEFAULT;
        return bbEOK;
    }

    bbCHAR** pList = bbStrSplit(pStr, (bbU32)',' | bbSTRSPLIT_STRIPSPACE | bbSTRSPLIT_INPLACE, &count);
    if (!pList)
        return bbELAST;

    ptFontInfo info;
    info.Init();

    if (count == 0)
        goto ptFontInfo_FromStr_err;

    type = bbStrToU32(pList[0], bbSTROPT_ALLFMT);
    info.mType = (bbU8)type;
    switch(type)
    {
    case ptFONTTYPE_MEM:
        if (count < 2)
            goto ptFontInfo_FromStr_err;
        info.mpFont = bbStrDup(pList[1]);
        break;

    case ptFONTTYPE_WIN:
        if (count < 7)
            goto ptFontInfo_FromStr_err;
        info.mpFont   = bbStrDup(pList[1]);
        info.mFormat  = (bbU8)bbStrToU32(pList[2], bbSTROPT_ALLFMT);
        info.mWidth   = (bbU8)bbStrToU32(pList[3], bbSTROPT_ALLFMT);
        info.mHeight  = (bbU8)bbStrToU32(pList[4], bbSTROPT_ALLFMT);
        info.mStyle   = (bbU8)bbStrToU32(pList[5], bbSTROPT_ALLFMT);
        info.mCharset = (bbU8)bbStrToU32(pList[6], bbSTROPT_ALLFMT);
        break;

    case ptFONTTYPE_DEFAULT:
        break;

    default:
        goto ptFontInfo_FromStr_err;
    }

    bbMemSwap(&info, this, sizeof(ptFontInfo));
    err = bbEOK;
    ptFontInfo_FromStr_err:
    if (err != bbEOK)
        bbErrSet(bbESYNTAX);
    bbMemFree(pList);
    info.Destroy();
    return err;
}

ptFont::ptFont()
{
    mInfo.Init();
    mRefCount = 0;
    mUkCP = mUkCPW = 0;
    mpBankUkCP = mpBankUkCPW = NULL;
    bbMemClear(mpPages, sizeof(mpPages)); // clear all pages to NULL -> not existent
}

ptFont::~ptFont()
{
    bbASSERT(mRefCount == 0);
    Clear();
}

void ptFont::Clear()
{
    mInfo.Destroy();

    #if (bbSIZEOF_CHARCP > 2)
    int i=16;
    #else
    int i=0;
    #endif

    do
    {
        void** pPage = (void**) mpPages[i];

        if (pPage >= (void**)4)
        {
            for (int j = ptFONT_BANKCOUNT-1; j>=0; --j)
            {
                if (pPage[j] >= (void*)4) bbMemFree(pPage[j]);
            }

            bbMemFree(pPage);
        }
    } while (--i >= 0);

    mUkCP = mUkCPW = 0;
    mpBankUkCP = mpBankUkCPW = NULL;
    bbMemClear(mpPages, sizeof(mpPages));
}

const bbU8* const * ptFont::LoadPage(bbCHARCP const /*cp*/)
{
    bbErrSet(bbENOTSUP);
    return NULL;
}

const bbU8* ptFont::LoadBank(bbCHARCP const /*cp*/)
{
    bbErrSet(bbENOTSUP);
    return NULL;
}

const bbU8* ptFont::GetBankNoReplace(bbCHARCP const cp)
{
    bbASSERT((bbU32)cp < 0x110000UL);

    #if (bbSIZEOF_CHARCP > 2)

    const bbU8* const * pPage = mpPages[cp>>16];

    if (pPage == 0)
    {
        bbErrSet(bbEUK);
        return NULL;
    }

    if (pPage == (const bbU8* const *)1)
    {
        if ((pPage = LoadPage(cp)) == 0)
            return NULL;
    }

    const bbU8* pBank = ((const bbU8*const*)pPage)[(cp>>ptFONT_BANKSIZE_LOG2) & (ptFONT_BANKCOUNT-1)];

    #else

    const bbU8* const * const pPage = mpPages[0];
    bbASSERT(pPage >= (bbU8*)4);

    const bbU8* pBank = ((const bbU8*const*)pPage)[(cp>>ptFONT_BANKSIZE_LOG2)];

    #endif

    if (pBank == 0)
    {
        bbErrSet(bbEUK);
        return NULL;
    }

    if (pBank == (const bbU8*)1)
    {
        if ((pBank = LoadBank(cp)) == 0)
            return NULL;
    }

    bbU32 const tmp = *((const bbU32*)pBank + (cp & (ptFONT_BANKSIZE-1)));
    if (tmp == 0)
    {
        bbErrSet(bbEUK);
        return NULL;
    }

    return pBank;
}

bbERR ptFont::SetUnknownCP(const bbCHARCP cp)
{
    const bbU8* pBank = GetBankNoReplace(cp);
    if (!pBank)
        return bbELAST;

    mUkCP = cp;
    mpBankUkCP = pBank;

    return bbEOK;
}

bbERR ptFont::SetUnknownCPW(const bbCHARCP cp)
{
    const bbU8* pBank = GetBankNoReplace(cp);
    if (!pBank)
        return bbELAST;

    mUkCPW = cp;
    mpBankUkCPW = pBank;

    return bbEOK;
}

void ptFont::GetGlyph(bbCHARCP cp, ptFontGlyph* const pGlyph)
{
    pGlyph->opt = 0;

    const bbU8* pBank = GetBankNoReplace(cp);

    if (!pBank)
    {
        if (bbErrGet() == bbEUK)
            pGlyph->opt = ptFONTGLYPH_OPT_UKCP;

        // remap code point if not existent or bank load error

        #if (bbCPG == bbCPG_UNICODE) || (bbCPG == bbCPG_UCS)
        if (GetWidth() && bbCpgUnicode_IsWide(cp))
        {
            cp = mUkCPW;
            pBank = mpBankUkCPW;
        }
        else
        #endif
        {
            cp = mUkCP;
            pBank = mpBankUkCP;
        }
    }

    bbU32 tmp = *((bbU32*)pBank + (cp & (ptFONT_BANKSIZE-1)));
    bbASSERT(tmp);

    pGlyph->pBmp = pBank + (tmp &~ 0xFF000000UL);
    pGlyph->width = (bbU8)(tmp >> 26);
}

/*
bbUINT ptFont::PrepareText(const bbCHAR* const pText, const bbUINT len, ptFontGlyph* const pGlyphs)
{
    bbUINT cu_rd = 0;
    bbUINT gl_wr = 0;

    for (;;)
    {
        if (cu_rd >= len) return gl_wr; // all chars processed, return number of written glyphs

        // - parse next code point
        bbCHARCP cp;
        bbCP_NEXT(pText, cu_rd, cp)

        GetGlyph(cp, &pGlyphs[gl_wr++]);
    }
}
*/

bbU8* ptFont::EditCP(const bbCHARCP cp, bbUINT width)
{
    bbU8** pPage = (bbU8**)mpPages[cp>>16];


    // get page

    if (pPage == (bbU8**)1)
    {
        if ((pPage = (bbU8**)LoadPage(cp)) == 0)
            return NULL;
    }

    if (pPage == 0) // page not existent in font -> patch and create it
    {
        if ((pPage = (bbU8**) bbMemAlloc( ptFONT_BANKCOUNT * sizeof(void*))) == NULL)
            return NULL;
        bbMemClear( pPage, ptFONT_BANKCOUNT * sizeof(void*));
        mpPages[cp>>16] = pPage;
    }

    // get bank

    bbUINT const bank = (cp >> ptFONT_BANKSIZE_LOG2) & (ptFONT_BANKCOUNT-1);
    bbU8* pBank = pPage[bank];

    if (pBank == (bbU8*)1)
    {
        if ((pBank = (bbU8*)LoadBank(cp)) == 0)
            return NULL;
    }

    if (GetWidth() && bbCpgUnicode_IsWide(cp))
        width<<=1;
    bbUINT charsize = width >> 3;
    if (width & 7) charsize++;
    charsize = charsize * GetHeight();

    if (pBank == 0) // bank not existent in font -> create it
    {
        if ((pBank = (bbU8*) bbMemAlloc(ptFONT_BANKSIZE * sizeof(bbU32) + charsize))==NULL)
            return NULL;
        bbMemClear(pBank, ptFONT_BANKSIZE * sizeof(bbU32));
        pPage[bank] = pBank;
    }

    // get code point

    bbU32 tmp = *((bbU32*)pBank + (cp & (ptFONT_BANKSIZE-1)));

    if (tmp == 0) // code point not existent -> create it
    {
        // realloc block
        bbU32 max = ptFONT_BANKSIZE * sizeof(bbU32);
        for (bbUINT i=0; i<ptFONT_BANKSIZE; i++)
        {
            tmp = *((bbU32*)pBank + i);
            tmp = (tmp &~ 0xFF000000UL) + GetHeight() * (tmp>>(26+3)) + (tmp&(7<<26) ? 1 : 0);
            if (tmp > max) max=tmp;
        }

        if (bbMemRealloc(max + charsize, (void**)&pBank) != bbEOK)
            return NULL;
        pPage[bank] = pBank;

        bbMemSet(pBank + max, 0xFF, charsize);

        tmp = max | (width << 26);
        *((bbU32*)pBank + (cp & (ptFONT_BANKSIZE-1))) = tmp;
    }

    return pBank + (tmp &~ 0xFF000000UL);
}

