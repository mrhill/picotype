#include "babel/mem.h"
#include "babel/str.h"
#include "babel/file.h"
#include "babel/log.h"

#include "ptFontMan.h"
#include "ptFont_mem.h"
#include "ptFont_win.h"

ptFontMan ptgFontMan;

bbERR ptFontMan::Init()
{
    mFonts.Init();
    mpFontDir = NULL;
    mhDefaultFont = (bbUINT)-1;
  
    return bbEOK;
}

void ptFontMan::Destroy()
{
#ifdef bbDEBUG
    for (bbUINT q=0; q<mFonts.GetSize(); q++)
    {
        bbASSERT(mFonts[q] == NULL);
    }
#endif

    bbMemFreeNull((void**)&mpFontDir);
    mFonts.Destroy();
}

bbERR ptFontMan::SetFontDir(const bbCHAR* const pDir)
{
    bbCHAR* pTmp = bbPathDelim(pDir);
    if (!pTmp)
        return bbELAST;
    bbMemFreeNull((void**)&mpFontDir);
    mpFontDir = pTmp;
    return bbEOK;
}

bbUINT ptFontMan::AddFont(ptFont* const pFont)
{
    bbUINT i;

    for (i=0; i < (int)mFonts.GetSize(); i++)
    {
        if (mFonts[i] == NULL)
            goto ptFontMan_AddFont_foundfree;
    }

    if (!mFonts.Grow(1))
        return (bbUINT)-1;

    mFonts[i] = NULL;

    ptFontMan_AddFont_foundfree:
    bbASSERT(mFonts[i] == NULL);

    mFonts[i] = pFont;
    return i;
}

bbUINT ptFontMan::GetFont(const ptFontInfo* const pInfo)
{
    ptFont* pFont = NULL;
    bbCHAR* pPath;
    bbERR   err;
    bbUINT  hFont;

    ptFONTTYPE const type = pInfo ? (ptFONTTYPE)pInfo->mType : ptFONTTYPE_DEFAULT;

    switch (type)
    {
    case ptFONTTYPE_MEM:
        if ((pFont = new ptFont_mem) == NULL)
        {
            bbErrSet(bbENOMEM);
            goto ptFontMan_GetFont_err;
        }

        if (!(pPath = bbStrDupEx(ptgFontMan.mpFontDir, bbStrLen(pInfo->mpFont))))
            goto ptFontMan_GetFont_err;
        bbStrCpy(pPath + bbStrLen(pPath), pInfo->mpFont);

        err = ((ptFont_mem*)pFont)->Load(pPath);
        if (err != bbEOK)
            bbLog(bbErr, bbT("ptFontMan::GetFont error %d loading '%s'\n"), err, pPath);

        bbMemFreeNull((void**)&pPath);

        if (err != bbEOK)
            goto ptFontMan_GetFont_err;

        break;
#if  ptUSE_FONTWIN == 1
    case ptFONTTYPE_WIN:
        if ((pFont = new ptFont_win) == NULL)
        {
            bbErrSet(bbENOMEM);
            goto ptFontMan_GetFont_err;
        }

        if ((((ptFont_win*)pFont)->Init((ptFONTFORMAT)pInfo->mFormat,
                                        pInfo->mpFont, 
                                        pInfo->mWidth,
                                        pInfo->mHeight,
                                        (ptFONTSTYLE)pInfo->mStyle,
                                        pInfo->mCharset,
                                        pInfo->mFormat == ptFONTFORMAT_WINRASTER ? ptFONT_WIN_INIT_NOHSCALE : 0)) != bbEOK)
        {
            goto ptFontMan_GetFont_err;
        }

        break;
#endif
    case ptFONTTYPE_DEFAULT:
        bbASSERT(mFonts.GetSize() && mFonts[0] && (mhDefaultFont!=(bbUINT)-1));
        hFont = mhDefaultFont;
        goto ptFontMan_GetFont_default;

    case ptFONTTYPE_COMPOSITE:
    default:
        bbErrSet(bbEBADPARAM);
        goto ptFontMan_GetFont_err;
    }

    if ((hFont = AddFont(pFont)) == (bbUINT)-1)
        goto ptFontMan_GetFont_err;

    ptFontMan_GetFont_default:

    mFonts[hFont]->mRefCount++;
    return hFont;

    ptFontMan_GetFont_err:
    delete pFont;
    return (bbUINT)-1;
}

void ptFontMan::ReleaseFont(bbUINT const hFont)
{
    if (hFont == (bbUINT)-1)
        return;

    bbASSERT(hFont < mFonts.GetSize());
    bbASSERT(mFonts[hFont]->mRefCount);

    if (--mFonts[hFont]->mRefCount == 0)
    {
        delete mFonts[hFont];
        mFonts[hFont] = NULL;
    }
}

