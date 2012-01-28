#include <babel/str.h>
#include <babel/file.h>
#include "ptPal.h"
#include "ptGC.h" // for ptRGBPalMatch()

ptPal::ptPal(const bbU32* pExtRGB, bbUINT size, bbCHAR* pExtName)
{
    Init();
    mOpt = ptPALOPT_EXTRGB|ptPALOPT_EXTNAME;
    mpRGB = (bbU32*)pExtRGB;
    mpName = pExtName;
    mColCount = size;
}

void ptPal::Clear()
{
    bbUINT const opt = mOpt;
    if (!(opt & ptPALOPT_EXTNAME)) bbMemFreeNull((void**)&mpName);
    if (!(opt & ptPALOPT_EXTRGB)) bbMemFreeNull((void**)&mpRGB);

    bbMemClear(this, sizeof(ptPal));
}

ptPal& ptPal::operator=(const ptPal& other)
{
    Destroy();
    Init(other);
    return *this;
}

bbERR ptPal::Init(const ptPal& other)
{
    bbMemCpy(this, &other, sizeof(ptPal));

    if (other.mpRGB)
    {
        mpRGB = (bbU32*)bbMemAlloc(sizeof(bbU32) * other.mColCount);
        if (mpRGB)
            bbMemCpy(mpRGB, other.mpRGB, sizeof(bbU32) * other.mColCount);
    }

    if (other.mpName)
        mpName = bbStrDup(other.mpName);

    return bbEOK;
}

ptPal::ptPal(const ptPal& other)
{
    Init(other);
}

ptPal* ptPal::Create(bbU8* pSrc, ptPALFMT srcfmt, bbUINT size, bbCHAR* const pName)
{
    ptPal* pPal = new ptPal;
    if (!pPal)
        return NULL;

    if (bbEOK != pPal->Set(pSrc, srcfmt, size, pName))
    {
        delete pPal;
        return NULL;
    }

    return pPal;
}

bbERR ptPal::Set(bbU8* pSrc, ptPALFMT srcfmt, bbUINT size, bbCHAR* const pName)
{
    bbUINT i;

    Clear();

    if (size)
    {
        if ((mpRGB = (bbU32*)bbMemAlloc(sizeof(bbU32) * size)) == NULL)
            goto ptPal_Create_err;
    }

    mColCount = (bbU16)size;

    if (pSrc)
    {
        switch (srcfmt)
        {
        case ptPALFMT_NATIVE:
        #if bbCPUE == bbCPUE_LE
        case ptPALFMT_RAWRGBA:
        #endif
            bbMemCpy(mpRGB, pSrc, sizeof(bbU32) * size);
            break;

        #if bbCPUE == bbCPUE_BE
        case ptPALFMT_RAWRGBA:
            for(i=0; i<size; ++i)
                mpRGB[i] = pSrc[i<<2] | (pSrc[(i<<2)+1]<<8) | (pSrc[(i<<2)+2]<<16) | (pSrc[(i<<2)+3]<<24); 
            break;
        #endif

        case ptPALFMT_RAWBGRA:
            for(i=0; i<size; ++i)
                mpRGB[i] = (pSrc[i<<2]<<16) | (pSrc[(i<<2)+1]<<8) | pSrc[(i<<2)+2] | (pSrc[(i<<2)+3]<<24); 
            break;

        case ptPALFMT_NATIVE_U64:
            for(i=0; i<size; ++i)
                mpRGB[i] = (bbU32)((bbU64*)pSrc)[i];
            break;
        }
    }

    if (pName)
    {
        if ((mpName = bbStrDup(pName)) == NULL)
            goto ptPal_Create_err;
    }

    return bbEOK;

    ptPal_Create_err:
    Destroy();
    return bbELAST;
}

void ptPal::SetAlpha(bbU8 alpha, bbUINT colStart, bbUINT colEnd)
{
    if (colEnd > mColCount)
        colEnd = mColCount;

    bbU32 a = (bbU32)alpha << 24;

    for (; colStart<colEnd; colStart++)
        mpRGB[colStart] = (mpRGB[colStart] & 0xFFFFFFUL) | a;
}

ptPal* ptPal::CreatePredefined(ptPALID const id)
{
    ptPal* pPal = new ptPal;
    if (!pPal)
        return NULL;

    if (bbEOK != pPal->SetPredefined(id))
    {
        delete pPal;
        return NULL;
    }

    return pPal;
}

bbERR ptPal::SetPredefined(ptPALID const id)
{
    bbUINT i;
    bbU32 r, g, b;

    Clear();

    if (bbEOK != SetColCount(256))
        return bbELAST;

    mOpt = ptPALOPT_EXTNAME;
    mFormat = ptPALFMT_NONE;

    bbU32* const pRGB = mpRGB;

    switch (id)
    {
    case ptPALID_GREY256:
        mpName = bbT("Grey256");
        for (i=0; i<256; i++)
        {
            pRGB[i] = i | (i<<8) | (i<<16) | 0xFF000000UL;
        }
        break;

    case ptPALID_RGB232:
        mpName = bbT("RGB232");
        for (i=0; i<256; i++)
        {
            r = i & 3;
            g = (i>>2) & 7;
            b = (i>>5) & 3;
            r = r | (b<<16);
            r = r | (r<<2);
            r = r | (r<<4);
            g = (g<<5) | (g<<1) | (g>>2);
            r = r | (g<<8);
            pRGB[i] = r | 0xFF000000UL;
        }
        break;

    default:
        bbASSERT(0); // unsupported id
        return bbErrSet(bbEBADPARAM);
    }

    return bbEOK;
}

bbERR ptPal::Save(const bbCHAR* pPath, ptPALFMT const fmt)
{
    int i;
    bbFILEH hFile = NULL;

    if ((bbUINT)((bbUINT)fmt - ptPALFMT_RAWRGB) >= (bbUINT)(ptPALFMTCOUNT-ptPALFMT_RAWRGB))
    {
        bbErrSet(bbEBADPARAM);
        goto ptPal_Save_err;
    }

    hFile = bbFileOpen(pPath, bbFILEOPEN_TRUNC | bbFILEOPEN_READWRITE);
    if (hFile == NULL)
        goto ptPal_Save_err;

    switch (fmt)
    {
    case ptPALFMT_RAWRGB:
    case ptPALFMT_ARCHIMEDES:

        for (i=0; i<(int)mColCount; i++)
        {
            if (fmt == ptPALFMT_ARCHIMEDES)
            {
                if (bbEOK != bbFileWriteLE(hFile, 0x100013UL | (i<<8), 3))
                    goto ptPal_Save_err;
            }

            if (bbEOK != bbFileWriteLE(hFile, mpRGB[i], 3))
                goto ptPal_Save_err;
        }

        break;
    }

    bbFileClose(hFile);
    return bbEOK;

    ptPal_Save_err:
    bbFileClose(hFile);
    return bbELAST;
}

bbERR ptPal::Load_Raw(bbU8* pBuf, bbU32 filesize)
{
    bbUINT colcount = filesize / 3;
    if (filesize - (colcount*3))
        return bbErrSet(bbEFILEFORMAT);

    if (SetColCount(colcount) != bbEOK)
        return bbELAST;

    bbU8* const pBufEnd = pBuf + colcount*3;
    bbU32* pRGB = mpRGB;
    while (pBuf < pBufEnd)
    {
        *(pRGB++) = bbLD24LE(pBuf) | 0xFF000000UL;
        pBuf += 3;
    }

    mFormat = ptPALFMT_RAWRGB;
    return bbEOK;
}

bbERR ptPal::Load_Archimedes(bbU8* pBuf, bbU32 filesize)
{
    bbUINT colcount = filesize / 6;
    if (filesize - (colcount*6))
        return bbErrSet(bbEFILEFORMAT);

    bbUINT maxcol = 0;
    bbU8* const pBufEnd = pBuf + colcount*6;
    while (pBuf < pBufEnd)
    {
        if (pBuf[0] != 0x13)
        {
            return bbErrSet(bbEFILEFORMAT);
        }

        if ((pBuf[2] == 0x10) && (pBuf[1] > maxcol))
        {
            maxcol = pBuf[1];
        }
        
        pBuf += 6;
    }
    pBuf -= colcount*6;

    if (SetColCount(maxcol+1) != bbEOK)
        return bbELAST;

    bbMemClear(mpRGB, sizeof(bbU32)*maxcol);

    while (pBuf < pBufEnd)
    {
        if (pBuf[2] == 0x10)
        {
            bbASSERT(pBuf[1] <= maxcol);
            mpRGB[pBuf[1]] = bbLD24LE(pBuf + 3)  | 0xFF000000UL;
        }
        pBuf += 6;
    }

    mFormat = ptPALFMT_ARCHIMEDES;
    return bbEOK;
}

bbERR ptPal::Load(const bbCHAR* pFilePath)
{
    bbFILEH  hFile = NULL;
    bbU8*    pBuf  = NULL;
    bbERR    err;
    bbU64    filesize;

    bbASSERT((mOpt & (ptPALOPT_EXTRGB|ptPALOPT_EXTNAME)) == 0);

    hFile = bbFileOpen(pFilePath, bbFILEOPEN_READ);
    if (hFile == NULL)
        goto ptPal_Load_err;

    //
    // - Load palette file into memory
    //
    if ((filesize = bbFileExt(hFile)) == (bbU32)-1)
        goto ptPal_Load_err;

    if ((filesize-3) >= (65536-3))
    {
        bbErrSet(bbEFILEFORMAT);
        goto ptPal_Load_err;
    }

    if ((pBuf = (bbU8*)bbMemAlloc((bbU32)filesize)) == NULL)
        goto ptPal_Load_err;

    if (bbEOK != bbFileRead(hFile, pBuf, (bbU32)filesize))
        goto ptPal_Load_err;

    bbFileClose(hFile);
    hFile = NULL;

    //
    // - Format detection
    //

    err = Load_Archimedes(pBuf, (bbU32)filesize);
    if (err == bbEOK)
        goto ptPal_Load_ok;
    if (err != bbEFILEFORMAT)
        goto ptPal_Load_err;

    err = Load_Raw(pBuf, (bbU32)filesize);
    if (err == bbEOK)
        goto ptPal_Load_ok;

    // - unknown format
    ptPal_Load_err:
    bbMemFree(pBuf);
    bbFileClose(hFile);
    return bbELAST;

    ptPal_Load_ok:

    bbMemFree(pBuf);

    //
    // - Optionally set palette name to filename
    //
    if (mpName == NULL)
    {
        bbPathSplit(pFilePath, NULL, &mpName, NULL);
    }

    return bbEOK;
}

bbERR ptPal::Marshal(bbArrU8* pBuf)
{
    bbU32 size = mColCount;
    size = 4 + (size<<2);

    bbUINT nameLen = mpName ? bbStrLen(mpName) : 0;
    size += 4 + nameLen * sizeof(bbCHAR);

    if (pBuf->SetSize(size) != bbEOK)
        return bbELAST;

    bbU8* p = pBuf->GetPtr();

    bbST16(p, mColCount); p+=2;
    *p++ = mOpt;
    *p++ = mFormat;
    bbST32(p, nameLen); p+=4;

    bbUINT i;
    for(i = 0; i < mColCount; i++)
    {
        bbU32 col = mpRGB[i];
        bbST32(p, col); p+=4;
    }

    for(i = 0; i < nameLen; i++)
    {
        bbST16(p, mpName[i]); p+=2;
    }

    return bbEOK;
}

bbU8* ptPal::Unmarshal(bbU8* p)
{
    Clear();

    bbUINT colCount = bbLD16(p); p+=2;

    mColCount = colCount;
    mOpt = *p++;
    mFormat = *p++;

    bbUINT nameLen = bbLD32(p); p+=4;

    mpRGB = (bbU32*)bbMemAlloc(colCount * 4);
    mpName = bbStrAlloc(nameLen);
    if (!mpRGB || !mpName)
        return NULL;

    for(bbUINT i = 0; i < mColCount; i++)
    {
        mpRGB[i] = bbLD32(p); p+=4;
    }

    bbStrNCpy(mpName, (bbCHAR*)p, nameLen);
    mpName[nameLen] = 0;
    p += nameLen;

    return p;
}

bbUINT ptPal::MatchRGBA(ptRGBA rgba) const
{
    const bbU32* const pRGB = mpRGB;
    bbUINT const palsize = mColCount;

    bbU32  bestdist = ptRGBDist(pRGB[0], rgba);
    bbU32  bestdista = ptRGBADistA(pRGB[0], rgba);
    bbUINT bestidx = 0;

    for (bbUINT i=1; i<palsize; ++i)
    {
        const bbU32 currgb = pRGB[i];
        const bbU32 dist   = ptRGBDist(currgb, rgba);
        const bbU32 dista  = ptRGBADistA(currgb, rgba);

        if (dist < bestdist)
        {
            bestdist = dist;
            bestdista = dista;
            bestidx = i;
        }
        else if (dist == bestdist)
        {
            if (dista < bestdista)
            {
                bestdist = dist;
                bestdista = dista;
                bestidx = i;
            }
        }
    }

    return bestidx;
}

void ptPal::ToYUV(const ptRGB2YUV& rgb2yuv)
{
    for(int i=0; i<(int)mColCount; i++)
        mpRGB[i] = rgb2yuv.ToYUVA(mpRGB[i]);
}

