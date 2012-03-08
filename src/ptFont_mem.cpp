#include "ptFont_mem.h"

ptFont_mem::ptFont_mem()
{
    mInfo.mType = ptFONTTYPE_MEM;
}

bbERR ptFont_mem::Load(const bbCHAR* const pFilename)
{
    #if (ptUSE_FONTZAP == 0) && (ptUSE_FONTBBC == 0)
    return bbErrSet(bbEFILEFORMAT);
    #else

    bbFILEH  fh;
    bbCHARCP ukcp;

    bbErrSet(bbEFILEFORMAT);
    Clear();

    mInfo.mType      = ptFONTTYPE_MEM;
    mInfo.mMonospace = 1;

    if ((fh = bbFileOpen( pFilename, bbFILEOPEN_READ)) == NULL)
        goto ptFont_mem_Load_err;

    #if ptUSE_FONTZAP == 1
    if ((ukcp = LoadZapFont(fh)) != (bbCHARCP)-1)
        goto ptFont_mem_Load_ok;
    if (bbErrGet() != bbEFILEFORMAT)
        goto ptFont_mem_Load_err;
    #endif

    #if ptUSE_FONTBBC == 1
    if (bbFileSeek(fh, 0, bbFILESEEK_SET) != bbEOK)
        goto ptFont_mem_Load_err;
    if ((ukcp = LoadBBCFont(fh)) != (bbCHARCP)-1)
        goto ptFont_mem_Load_ok;

    goto ptFont_mem_Load_err;
    #endif

    ptFont_mem_Load_ok:

    // set replacement for unknown CP
    if (SetUnknownCP('.') != bbEOK)
    {
        if (SetUnknownCP(ukcp) != bbEOK)
            goto ptFont_mem_Load_err;
    }

    if (SetUnknownCPW(0x25A1) != bbEOK)
    {
        bbU8* pCPWBmp = EditCP(0x25A1, GetWidth());
        if (pCPWBmp == NULL)
            goto ptFont_mem_Load_err;

        //xxx copy CPW bitmap here, return for now

        if (SetUnknownCPW(0x25A1) != bbEOK)
            goto ptFont_mem_Load_err;
    }

    // set font name
    bbCHAR* pExt;
    if (bbPathSplit(pFilename, NULL, &mInfo.mpName, &pExt) != bbEOK)
        goto ptFont_mem_Load_err;

    mInfo.mpFont = bbPathJoin(NULL, mInfo.mpName, pExt);
    bbMemFree(pExt);
    if (mInfo.mpFont == NULL)
        goto ptFont_mem_Load_err;

    bbFileClose(fh);
    return bbEOK;

    ptFont_mem_Load_err:

    bbFileClose(fh);
    Clear();
    return bbELAST;

    #endif // #if (ptUSE_FONTZAP == 0) && (ptUSE_FONTBBC == 0)
}

#if ptUSE_FONTZAP == 1

bbCHARCP ptFont_mem::LoadZapFont(bbFILEH const fh)
{
    bbUINT cp;
    bbU8** pBMP;
    bbU32  data_wr = 0;
    bbUINT cp_first;
    bbUINT cp_end;
    bbUINT charsize;

    bbU8 buf[0x20];
    #define pBuf (&buf[0])

    // load header

    if (bbFileRead(fh, pBuf, 0x20) != bbEOK)
        goto LoadZapFont_err;

    // check for "ZapFont",13 at start of file
    if (bbMemCmp(pBuf, "ZapFont\xD", 8) != 0)
    {
        bbErrSet(bbEFILEFORMAT);
        goto LoadZapFont_err;
    }

    // extract header
    mInfo.mWidth    = pBuf[0x2 * 4];
    mInfo.mHeight   = pBuf[0x3 * 4];
    mInfo.mCodepage = bbCPG_ISO8859_1; // Default to Latin 1
    mInfo.mFormat   = ptFONTFORMAT_ZAP;

    cp_first = (bbUINT) bbLDA16LE( pBuf + 0x4 * 4 ); // first used char
    cp_end   = (bbUINT) bbLDA16LE( pBuf + 0x5 * 4 ); // last used char + 1

    //if (cp_end > 256)  cp_end = 256; // ignore Zap cursors

    charsize = GetHeight() * (GetWidth() >> 3);
    if ((bbUINT)GetWidth() & 7) charsize += (bbUINT)GetHeight();

    // Zap fonts are always within the BMP (page 0), allocate its bank index
    if ((pBMP = (bbU8**) bbMemAlloc( ptFONT_BANKCOUNT * sizeof(void*))) == NULL)
        goto LoadZapFont_err;
    bbMemClear( pBMP, ptFONT_BANKCOUNT * sizeof(void*));
    mpPages[0] = pBMP;

    // loop through all code points contained in font

    for (cp = cp_first; cp < cp_end; cp++)
    {
        // ensure mem for character bank allocated

        if (pBMP[cp >> ptFONT_BANKSIZE_LOG2] == NULL)
        {
            if ((pBMP[cp >> ptFONT_BANKSIZE_LOG2] = (bbU8*) bbMemAlloc( ptFONT_BANKSIZE * (charsize + sizeof(bbU32))))==NULL)
                goto LoadZapFont_err;

            data_wr = (bbU32) ptFONT_BANKSIZE * sizeof(bbU32);

            bbMemClear(pBMP[cp >> ptFONT_BANKSIZE_LOG2], data_wr);
        }

        // load data for current char

        *((bbU32*)pBMP[cp >> ptFONT_BANKSIZE_LOG2] + (cp & (ptFONT_BANKSIZE-1))) = data_wr | (((bbU32)GetWidth()) << (32-6));

        if (bbFileRead(fh, pBMP[cp >> ptFONT_BANKSIZE_LOG2] + data_wr, charsize) != bbEOK)
            goto LoadZapFont_err;

        data_wr += charsize;
    }

    return cp_first;
    LoadZapFont_err:
    return (bbCHARCP)-1;

    #undef pBuf
}

#endif // #if ptUSE_FONTZAP == 1

#if ptUSE_FONTBBC == 1

bbCHARCP ptFont_mem::LoadBBCFont(bbFILEH const fh)
{
    #if ptFONT_BANKSIZE > 256
    #error adjust code
    #endif

    bbU8**  pBMP;
    bbU32   data_wr[256/ptFONT_BANKSIZE];
    bbUINT  numloops = 0;
    bbERR   err;
    bbU8    buf[12];

    if ((pBMP = (bbU8**) bbMemAlloc( ptFONT_BANKCOUNT * sizeof(void*))) == NULL)
        goto LoadBBCFont_err;
    bbMemClear( pBMP, ptFONT_BANKCOUNT * sizeof(void*));
    mpPages[0] = pBMP; // save now, so Clear() frees this on failure

    buf[2+1] = ' '; // force space to be existent in all BBC fonts
    *(bbU32*)&buf[4] = 0UL;
    *(bbU32*)&buf[8] = 0UL;

    for(;;)
    {
        bbUINT const cp   = (bbUINT) buf[2+1];
        bbUINT const bank = cp >> ptFONT_BANKSIZE_LOG2;

        if (pBMP[bank] == NULL)
        {
            if ((pBMP[bank] = (bbU8*) bbMemAlloc( ptFONT_BANKSIZE * (8 + sizeof(bbU32))))==NULL)
                goto LoadBBCFont_err;

            data_wr[bank] = (bbU32) ptFONT_BANKSIZE * sizeof(bbU32);

            bbMemClear(pBMP[cp >> ptFONT_BANKSIZE_LOG2], data_wr[bank]);
        }

        *((bbU32*)pBMP[cp >> ptFONT_BANKSIZE_LOG2] + (cp & (ptFONT_BANKSIZE-1))) = data_wr[bank] | (8 << (32-6));

        bbU8* const pTmp = pBMP[cp >> ptFONT_BANKSIZE_LOG2] + data_wr[bank];
        int i=4;
        do
        {
            register bbU32 pix = *(bbU32*)&buf[4+i];
            pix = ((pix>>1)&0x55555555UL) | ((pix&0x55555555UL)<<1);
            pix = ((pix>>2)&0x33333333UL) | ((pix&0x33333333UL)<<2);
            pix = ((pix>>4)&0x0F0F0F0FUL) | ((pix&0x0F0F0F0FUL)<<4);
            *(bbU32*)&pTmp[i] = pix;
        } while((i-=4)>=0);

        data_wr[bank] += 8;

        // read next char
        if ((err = bbFileRead(fh, &buf[2], 10)) != bbEOK)
        {
            if (err == bbEEOF) break;
            goto LoadBBCFont_err;
        }

        if ((int)buf[2+0] != 23) // magic number
            break; 

        numloops++;
    }

    // format not recognized ?
    if ((numloops == 0) || ((err == bbEOK) && (numloops < 4)))
    {
        bbErrSet(bbEFILEFORMAT);
        goto LoadBBCFont_err;
    }

    mInfo.mWidth    =
    mInfo.mHeight   = 8;
    mInfo.mCodepage = bbCPG_ISO8859_1; // RISC OS is Latin 1 (usually)
    mInfo.mFormat   = ptFONTFORMAT_BBC;

    return ' ';
    LoadBBCFont_err:
    return (bbCHARCP)-1;
}

#endif // #if ptUSE_FONTBBC == 1
