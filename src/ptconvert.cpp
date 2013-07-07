#include "ptconvert.h"
#include "ptPal.h"

void ptMakePal2PalLookup(const ptPal* pSrc, const ptPal* pDst, bbU8* pLU)
{
    bbUINT srcColCount = pSrc->GetColCount();

    for(bbUINT i=0; i<srcColCount; i++)
    {
        ptRGBA srcCol = pSrc->GetColRGBA(i);
        bbUINT dstBestIdx = pDst->MatchRGBA(srcCol);
        *pLU++ = (bbU8)dstBestIdx;
    }
}

static const bbU32 patternLSBLeft[16] =
{
    0x00000000UL,0x00000001UL,0x00000100UL,0x00000101UL,
    0x00010000UL,0x00010001UL,0x00010100UL,0x00010101UL,
    0x01000000UL,0x01000001UL,0x01000100UL,0x01000101UL,
    0x01010000UL,0x01010001UL,0x01010100UL,0x01010101UL
};

static const bbU32 patternMSBLeft[16] =
{
    0x00000000UL,0x01000000UL,0x00010000UL,0x01010000UL,
    0x00000100UL,0x01000100UL,0x00010100UL,0x01010100UL,
    0x00000001UL,0x01000001UL,0x00010001UL,0x01010001UL,
    0x00000101UL,0x01000101UL,0x00010101UL,0x01010101UL,
};

void ptExpand_1BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder)
{
    if (srcOrder==ptBITORDER_LSBLEFT)
    {
        while(width>=8)
        {
            register bbUINT pixels = *pSrc++;
            register bbU32 a = patternLSBLeft[pixels & 15]; bbST32LE(pDst, a); pDst+=4;
                           a = patternLSBLeft[pixels >> 4]; bbST32LE(pDst, a); pDst+=4;
            width-=8;
        }
    }
    else
    {
        while(width>=8)
        {
            register bbUINT pixels = *pSrc++;
            register bbU32 a = patternMSBLeft[pixels >> 4]; bbST32LE(pDst, a); pDst+=4;
                           a = patternMSBLeft[pixels & 15]; bbST32LE(pDst, a); pDst+=4;
            width-=8;
        }
    }

    if (width)
    {
        bbU32 pixelsLeft = *pSrc;

        if (srcOrder == ptBITORDER_LSBLEFT)
        {
            do
            {
                *pDst++ = (bbU8)(pixelsLeft & 1);
                pixelsLeft>>=1;
            } while(--width);
        }
        else
        {
            pixelsLeft<<=24;
            do
            {
                *pDst++ = pixelsLeft>>31;
                pixelsLeft<<=1;
            } while(--width);
        }
    }
}

void ptMerge_1BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder, bbUINT shift)
{
    if (srcOrder==ptBITORDER_LSBLEFT)
    {
        while(width>=8)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ |= (pixels&1) << shift;
            *pDst++ |= ((pixels>>1)&1) << shift;
            *pDst++ |= ((pixels>>2)&1) << shift;
            *pDst++ |= ((pixels>>3)&1) << shift;
            *pDst++ |= ((pixels>>4)&1) << shift;
            *pDst++ |= ((pixels>>5)&1) << shift;
            *pDst++ |= ((pixels>>6)&1) << shift;
            *pDst++ |= (pixels>>7) << shift;
            width-=8;
        }
    }
    else
    {
        while(width>=8)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ |= (pixels>>7) << shift;
            *pDst++ |= ((pixels>>6)&1) << shift;
            *pDst++ |= ((pixels>>5)&1) << shift;
            *pDst++ |= ((pixels>>4)&1) << shift;
            *pDst++ |= ((pixels>>3)&1) << shift;
            *pDst++ |= ((pixels>>2)&1) << shift;
            *pDst++ |= ((pixels>>1)&1) << shift;
            *pDst++ |= (pixels&1) << shift;
            width-=8;
        }
    }

    if (width)
    {
        bbU32 pixelsLeft = *pSrc;

        if (srcOrder == ptBITORDER_LSBLEFT)
        {
            do
            {
                *pDst++ |= (bbU8)((pixelsLeft & 1)<<shift);
                pixelsLeft>>=1;
            } while(--width);
        }
        else
        {
            pixelsLeft<<=24;
            do
            {
                *pDst++ |= (pixelsLeft>>31)<<shift;
                pixelsLeft<<=1;
            } while(--width);
        }
    }
}

void ptExpand_2BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder)
{
    if (srcOrder == ptBITORDER_LSBLEFT)
    {
        while(width>=4)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ = pixels & 3;
            *pDst++ = (pixels>>2) & 3;
            *pDst++ = (pixels>>4) & 3;
            *pDst++ = pixels>>6;
            width-=4;
        }

        if (width)
        {
            bbUINT pixelsLeft = *pSrc;
            do
            {
                *pDst++ = (bbU8)(pixelsLeft & 3);
                pixelsLeft>>=2;
            } while(--width);
        }
    }
    else
    {
        while(width>=4)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ = pixels>>6;
            *pDst++ = (pixels>>4) & 3;
            *pDst++ = (pixels>>2) & 3;
            *pDst++ = pixels & 3;
            width-=4;
        }

        if (width)
        {
            bbU32 pixelsLeft = (bbU32)*pSrc<<24;
            do
            {
                *pDst++ = (bbU8)(pixelsLeft >> 30);
                pixelsLeft<<=2;
            } while(--width);
        }
    }
}

void ptExpand_4BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder)
{
    if (srcOrder == ptBITORDER_LSBLEFT)
    {
        while(width>=2)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ = pixels & 15;
            *pDst++ = (pixels>>4) & 15;
            width-=2;
        }

        if (width)
            *pDst = *pSrc & 15;
    }
    else
    {
        while(width>=2)
        {
            register bbUINT pixels = *pSrc++;
            *pDst++ = (pixels>>4) & 15;
            *pDst++ = pixels & 15;
            width-=2;
        }

        if (width)
            *pDst = (bbU8)(((bbUINT)*pSrc >> 4) & 15);
    }
}

void ptConvert_8BppTo1BppLSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=8)
    {
        register bbU32 b = pColIdxLU[*pSrc++];
        b |= (bbU32)pColIdxLU[*pSrc++] << 1;
        b |= (bbU32)pColIdxLU[*pSrc++] << 2;
        b |= (bbU32)pColIdxLU[*pSrc++] << 3;
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++] << 5;
        b |= (bbU32)pColIdxLU[*pSrc++] << 6;
        b |= (bbU32)pColIdxLU[*pSrc++] << 7;
        *pDst++ = (bbU8)b;
        width-=8;
    }

    if (width)
    {
        bbUINT shift = 0;
        register bbU32 pix = 0;

        if (width>=4)
        {
            pix = (bbU32)pColIdxLU[*pSrc++];
            pix |= (bbU32)pColIdxLU[*pSrc++] << 1;
            pix |= (bbU32)pColIdxLU[*pSrc++] << 2;
            pix |= (bbU32)pColIdxLU[*pSrc++] << 3;
            width-=4;
            shift=4;
        }

        if (width>=2)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 1;
            pix |= b<<shift;
            width-=2;
            shift+=2;
        }

        if (width)
            pix |= pColIdxLU[*pSrc] << shift;

        *pDst = (bbU8)pix;
    }
}

void ptConvert_8BppTo1BppMSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=8)
    {
        register bbU32 b = pColIdxLU[*pSrc++] << 7;
        b |= (bbU32)pColIdxLU[*pSrc++] << 6;
        b |= (bbU32)pColIdxLU[*pSrc++] << 5;
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++] << 3;
        b |= (bbU32)pColIdxLU[*pSrc++] << 2;
        b |= (bbU32)pColIdxLU[*pSrc++] << 1;
        b |= (bbU32)pColIdxLU[*pSrc++];
        *pDst++ = (bbU8)b;
        width-=8;
    }

    if (width)
    {
        bbUINT shift = 6;
        register bbU32 pix = 0;

        if (width>=4)
        {
            pix = (bbU32)pColIdxLU[*pSrc++] << 7;
            pix |= (bbU32)pColIdxLU[*pSrc++] << 6;
            pix |= (bbU32)pColIdxLU[*pSrc++] << 5;
            pix |= (bbU32)pColIdxLU[*pSrc++] << 4;
            width-=4;
            shift=2;
        }

        if (width>=2)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++] << 1;
            b |= (bbU32)pColIdxLU[*pSrc++];
            pix |= b<<shift;
            width-=2;
            shift-=2;
        }

        if (width)
            pix |= (pColIdxLU[*pSrc]<<1) << shift;

        *pDst = (bbU8)pix;
    }
}

void ptConvert_8BppTo2BppLSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=16)
    {
        register bbU32 b = pColIdxLU[*pSrc++];
        b |= (bbU32)pColIdxLU[*pSrc++] << 2;
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++] << 6;
        b |= (bbU32)pColIdxLU[*pSrc++] << 8;
        b |= (bbU32)pColIdxLU[*pSrc++] << 10;
        b |= (bbU32)pColIdxLU[*pSrc++] << 12;
        b |= (bbU32)pColIdxLU[*pSrc++] << 14;
        b |= (bbU32)pColIdxLU[*pSrc++] << 16;
        b |= (bbU32)pColIdxLU[*pSrc++] << 18;
        b |= (bbU32)pColIdxLU[*pSrc++] << 20;
        b |= (bbU32)pColIdxLU[*pSrc++] << 22;
        b |= (bbU32)pColIdxLU[*pSrc++] << 24;
        b |= (bbU32)pColIdxLU[*pSrc++] << 26;
        b |= (bbU32)pColIdxLU[*pSrc++] << 28;
        b |= (bbU32)pColIdxLU[*pSrc++] << 30;
        bbST32LE(pDst, b); pDst+=4;
        width-=16;
    }

    if (width)
    {
        if (width>=8)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 2;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++] << 6;
            b |= (bbU32)pColIdxLU[*pSrc++] << 8;
            b |= (bbU32)pColIdxLU[*pSrc++] << 10;
            b |= (bbU32)pColIdxLU[*pSrc++] << 12;
            b |= (bbU32)pColIdxLU[*pSrc++] << 14;
            bbST16LE(pDst, b); pDst+=2;
            width-=8;
        }

        if (width>=4)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 2;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++] << 6;
            *pDst++=(bbU8)b;
            width-=4;
        }

        if (width>=2)
        {
            register bbU32 b = pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 2;
            if (width>2)
                b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            *pDst = (bbU8)b;
        }
        else if (width)
        {
            *pDst = pColIdxLU[*pSrc];
        }
    }
}

void ptConvert_8BppTo2BppMSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=16)
    {
        register bbU32 b = pColIdxLU[*pSrc++]<<30;
        b |= (bbU32)pColIdxLU[*pSrc++] << 28;
        b |= (bbU32)pColIdxLU[*pSrc++] << 26;
        b |= (bbU32)pColIdxLU[*pSrc++] << 24;
        b |= (bbU32)pColIdxLU[*pSrc++] << 22;
        b |= (bbU32)pColIdxLU[*pSrc++] << 20;
        b |= (bbU32)pColIdxLU[*pSrc++] << 18;
        b |= (bbU32)pColIdxLU[*pSrc++] << 16;
        b |= (bbU32)pColIdxLU[*pSrc++] << 14;
        b |= (bbU32)pColIdxLU[*pSrc++] << 12;
        b |= (bbU32)pColIdxLU[*pSrc++] << 10;
        b |= (bbU32)pColIdxLU[*pSrc++] << 8;
        b |= (bbU32)pColIdxLU[*pSrc++] << 6;
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++] << 2;
        b |= (bbU32)pColIdxLU[*pSrc++];
        bbST32BE(pDst, b); pDst+=4;
        width-=16;
    }

    if (width)
    {
        if (width>=8)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++]<<14;
            b |= (bbU32)pColIdxLU[*pSrc++] << 12;
            b |= (bbU32)pColIdxLU[*pSrc++] << 10;
            b |= (bbU32)pColIdxLU[*pSrc++] << 8;
            b |= (bbU32)pColIdxLU[*pSrc++] << 6;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++] << 2;
            b |= (bbU32)pColIdxLU[*pSrc++];
            bbST16BE(pDst, b); pDst+=2;
            width-=8;
        }

        if (width>=4)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++]<<6;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++] << 2;
            b |= (bbU32)pColIdxLU[*pSrc++];
            *pDst++=(bbU8)b;
            width-=4;
        }

        if (width>=2)
        {
            register bbU32 b = pColIdxLU[*pSrc++]<<6;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            if (width>2)
                b |= (bbU32)pColIdxLU[*pSrc] << 2;
            *pDst = (bbU8)b;
        }
        else if (width)
        {
            *pDst = pColIdxLU[*pSrc]<<6;
        }
    }
}

void ptConvert_8BppTo4BppLSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=8)
    {
        register bbU32 b = pColIdxLU[*pSrc++];
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++] << 8;
        b |= (bbU32)pColIdxLU[*pSrc++] << 12;
        b |= (bbU32)pColIdxLU[*pSrc++] << 16;
        b |= (bbU32)pColIdxLU[*pSrc++] << 20;
        b |= (bbU32)pColIdxLU[*pSrc++] << 24;
        b |= (bbU32)pColIdxLU[*pSrc++] << 28;
        bbST32LE(pDst, b); pDst+=4;
        width-=8;
    }

    if (width)
    {
        if (width>=4)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++] << 8;
            b |= (bbU32)pColIdxLU[*pSrc++] << 12;
            bbST16LE(pDst, b); pDst+=2;
            width-=4;
        }

        if (width>=2)
        {
            register bbU32 b = pColIdxLU[*pSrc++];
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            *pDst++=(bbU8)b;
            width-=2;
        }

        if (width)
            *pDst = pColIdxLU[*pSrc];
    }
}

void ptConvert_8BppTo4BppMSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=8)
    {
        register bbU32 b = (bbU32)pColIdxLU[*pSrc++]<<28;
        b |= (bbU32)pColIdxLU[*pSrc++] << 24;
        b |= (bbU32)pColIdxLU[*pSrc++] << 20;
        b |= (bbU32)pColIdxLU[*pSrc++] << 16;
        b |= (bbU32)pColIdxLU[*pSrc++] << 12;
        b |= (bbU32)pColIdxLU[*pSrc++] << 8;
        b |= (bbU32)pColIdxLU[*pSrc++] << 4;
        b |= (bbU32)pColIdxLU[*pSrc++];
        bbST32BE(pDst, b); pDst+=4;
        width-=8;
    }

    if (width)
    {
        if (width>=4)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++]<<12;
            b |= (bbU32)pColIdxLU[*pSrc++] << 8;
            b |= (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++];
            bbST16BE(pDst, b); pDst+=2;
            width-=4;
        }

        if (width>=2)
        {
            register bbU32 b = (bbU32)pColIdxLU[*pSrc++] << 4;
            b |= (bbU32)pColIdxLU[*pSrc++];
            *pDst++=(bbU8)b;
            width-=2;
        }

        if (width)
            *pDst = pColIdxLU[*pSrc]<<4;
    }
}

void ptConvert_8BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU)
{
    while(width>=4)
    {
        register bbU32 b = pColIdxLU[*pSrc++];
        b |= pColIdxLU[*pSrc++] << 8;
        b |= pColIdxLU[*pSrc++] << 16;
        b |= pColIdxLU[*pSrc++] << 24;
        bbST32LE(pDst, b); pDst+=4;
        width-=4;
    }

    if (width)
    {
        if (width>=2)
        {
            register bbU32 b = pColIdxLU[*pSrc++];
            b |= pColIdxLU[*pSrc++] << 8;
            bbST16LE(pDst, b); pDst+=2;
            width-=2;
        }

        if (width)
            *pDst = pColIdxLU[*pSrc];
    }
}

void ptConvert_YUV420ToRGB888(const bbU8* pSrcY0,
                              const bbU8* pSrcY1,
                              const bbU8* pSrcU,
                              const bbU8* pSrcV,
                              bbU8* pDst,
                              bbU8* pDst2,
                              bbU32 width,
                              const bbS16* pYUV2RGB)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<1) + width;

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
            int const y1 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
            int const u  = ((int)*(pSrcU++) + (int)pYUV2RGB[1]);
            int const v  = ((int)*(pSrcV++) + (int)pYUV2RGB[2]);

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3] = p; // R1
            pDst += 6;
        }

        pSrcU -= width>>1;
        pSrcV -= width>>1;
        pSrcY = pSrcY1;
        pDst = pDst2;

    } while (pSrcY && --i);
}

void ptConvert_YUV420ToRGBA8888(const bbU8* pSrcY0,
                                const bbU8* pSrcY1,
                                const bbU8* pSrcU,
                                const bbU8* pSrcV,
                                bbU8* pDst,
                                bbU32 width,
                                const bbS16* pYUV2RGB)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<2);

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
            int const y1 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
            int const u  = ((int)*(pSrcU++) + (int)pYUV2RGB[1]);
            int const v  = ((int)*(pSrcV++) + (int)pYUV2RGB[2]);

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4] = p; // R1
            pDst[3] = pDst[4+3] = 255;
            pDst += 8;
        }

        pSrcU -= width>>1;
        pSrcV -= width>>1;
        pSrcY = pSrcY1;

    } while (pSrcY && --i);
}

void ptConvert_YUV42016ToRGB888(const bbU8* pSrcY0,
                                const bbU8* pSrcY1,
                                const bbU8* pSrcU,
                                const bbU8* pSrcV,
                                bbU8* pDst,
                                bbU8* pDst2,
                                bbU32 width,
                                const bbS16* pYUV2RGB,
                                unsigned shift,
                                ptENDIAN endian)
{
    // converts 2 lines
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<1) + width;

        if (endian == ptENDIAN_LE)
        {
            while (pDst < pDstEnd)
            {
                int const y0 = ((int)(bbLD16LE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
                int const y1 = ((int)(bbLD16LE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
                int const u  = ((int)(bbLD16LE(pSrcU)>>shift) + (int)pYUV2RGB[1]); pSrcU+=2;
                int const v  = ((int)(bbLD16LE(pSrcV)>>shift) + (int)pYUV2RGB[2]); pSrcV+=2;

                int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                register int p;
                if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[2] = p; // B0
                if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3+2] = p; // B1
                tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[1] = p; // G0
                if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3+1] = p; // G1
                tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[0] = p; // R0
                if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3] = p; // R1
                pDst += 6;
            }
        }
        else
        {
            while (pDst < pDstEnd)
            {
                int const y0 = ((int)(bbLD16BE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
                int const y1 = ((int)(bbLD16BE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
                int const u  = ((int)(bbLD16BE(pSrcU)>>shift) + (int)pYUV2RGB[1]); pSrcU+=2;
                int const v  = ((int)(bbLD16BE(pSrcV)>>shift) + (int)pYUV2RGB[2]); pSrcV+=2;

                int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
                register int p;
                if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[2] = p; // B0
                if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3+2] = p; // B1
                tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
                if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[1] = p; // G0
                if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3+1] = p; // G1
                tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
                if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[0] = p; // R0
                if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
                if (p>255) p=255;
                pDst[3] = p; // R1
                pDst += 6;
            }
        }

        pSrcU -= width;
        pSrcV -= width;
        pSrcY = pSrcY1;
        pDst = pDst2;

    } while (pSrcY && --i);
}

void ptConvert_YUV42016LEToRGBA8888(const bbU8* pSrcY0,
                                    const bbU8* pSrcY1,
                                    const bbU8* pSrcU,
                                    const bbU8* pSrcV,
                                    bbU8* pDst,
                                    bbU32 width,
                                    const bbS16* pYUV2RGB,
                                    unsigned shift)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<2);

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)(bbLD16LE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
            int const y1 = ((int)(bbLD16LE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
            int const u  = ((int)(bbLD16LE(pSrcU)>>shift) + (int)pYUV2RGB[1]); pSrcU+=2;
            int const v  = ((int)(bbLD16LE(pSrcV)>>shift) + (int)pYUV2RGB[2]); pSrcV+=2;

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4] = p; // R1
            pDst[3] = pDst[4+3] = 255;
            pDst += 8;
        }

        pSrcU -= width;
        pSrcV -= width;
        pSrcY = pSrcY1;

    } while (pSrcY && --i);
}

void ptConvert_YUV42016BEToRGBA8888(const bbU8* pSrcY0,
                                    const bbU8* pSrcY1,
                                    const bbU8* pSrcU,
                                    const bbU8* pSrcV,
                                    bbU8* pDst,
                                    bbU32 width,
                                    const bbS16* pYUV2RGB,
                                    unsigned shift)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<2);

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)(bbLD16BE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
            int const y1 = ((int)(bbLD16BE(pSrcY)>>shift) + (int)pYUV2RGB[0]); pSrcY+=2;
            int const u  = ((int)(bbLD16BE(pSrcU)>>shift) + (int)pYUV2RGB[1]); pSrcU+=2;
            int const v  = ((int)(bbLD16BE(pSrcV)>>shift) + (int)pYUV2RGB[2]); pSrcV+=2;

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4] = p; // R1
            pDst[3] = pDst[4+3] = 255;
            pDst += 8;
        }

        pSrcU -= width;
        pSrcV -= width;
        pSrcY = pSrcY1;

    } while (pSrcY && --i);
}

void ptConvert_YUVNV12ToRGBA8888(const bbU8* pSrcY0,
                                 const bbU8* pSrcY1,
                                 const bbU8* pSrcUV,
                                 bbU8* pDst,
                                 bbU32 width,
                                 const bbS16* pYUV2RGB)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<2);

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)*(pSrcY++)  + (int)pYUV2RGB[0]);
            int const y1 = ((int)*(pSrcY++)  + (int)pYUV2RGB[0]);
            int const u  = ((int)*(pSrcUV++) + (int)pYUV2RGB[1]);
            int const v  = ((int)*(pSrcUV++) + (int)pYUV2RGB[2]);

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[4] = p; // R1
            pDst[3] = pDst[4+3] = 255;
            pDst += 8;
        }

        pSrcUV -= width;
        pSrcY = pSrcY1;

    } while (pSrcY && --i);
}

void ptConvert_YUVNV12ToRGB888(const bbU8* pSrcY0,
                               const bbU8* pSrcY1,
                               const bbU8* pSrcUV,
                               bbU8* pDst,
                               bbU8* pDst2,
                               bbU32 width,
                               const bbS16* pYUV2RGB)
{
    // converts 2 lines
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<1) + width;

        while (pDst < pDstEnd)
        {
            int const y0 = ((int)*(pSrcY++)  + (int)pYUV2RGB[0]);
            int const y1 = ((int)*(pSrcY++)  + (int)pYUV2RGB[0]);
            int const u  = ((int)*(pSrcUV++) + (int)pYUV2RGB[1]);
            int const v  = ((int)*(pSrcUV++) + (int)pYUV2RGB[2]);

            int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
            register int p;
            if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B0
            if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3+2] = p; // B1
            tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
            if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G0
            if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3+1] = p; // G1
            tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
            if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R0
            if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
            if (p>255) p=255;
            pDst[3] = p; // R1
            pDst += 6;
        }

        pSrcUV -= width;
        pSrcY = pSrcY1;
        pDst = pDst2;

    } while (pSrcY && --i);
}

void ptConvert_YUYVToRGB888(const bbU8* pSrcY,
                            bbU8* pDst,
                            bbU32 width,
                            const bbS16* pYUV2RGB)
{
    // converts 1 line
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const y0 = ((int)pSrcY[0] + pYUV2RGB[0]);
        int const u  = ((int)pSrcY[1] + pYUV2RGB[1]);
        int const y1 = ((int)pSrcY[2] + pYUV2RGB[0]);
        int const v  = ((int)pSrcY[3] + pYUV2RGB[2]);
        pSrcY += 4;

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3] = p; // R1
        pDst += 6;
    }
}

 void ptConvert_YUYVToRGBA8888(const bbU8* pSrcY,
                              bbU8* pDst,
                              bbU32 width,
                              const bbS16* pYUV2RGB)
{
    // converts 1 line, dst stride is width*4
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const y0 = ((int)pSrcY[0] + pYUV2RGB[0]);
        int const u  = ((int)pSrcY[1] + pYUV2RGB[1]);
        int const y1 = ((int)pSrcY[2] + pYUV2RGB[0]);
        int const v  = ((int)pSrcY[3] + pYUV2RGB[2]);
        pSrcY += 4;

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4] = p; // R1
        pDst[3] = pDst[4+3] = 255;
        pDst += 8;
    }
}

void ptConvert_UYVYToRGB888(const bbU8* pSrcY,
                            bbU8* pDst,
                            bbU32 width,
                            const bbS16* pYUV2RGB)
{
    // converts 1 line
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const u  = ((int)pSrcY[0] + pYUV2RGB[1]);
        int const y0 = ((int)pSrcY[1] + pYUV2RGB[0]);
        int const v  = ((int)pSrcY[2] + pYUV2RGB[2]);
        int const y1 = ((int)pSrcY[3] + pYUV2RGB[0]);
        pSrcY += 4;

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3] = p; // R1
        pDst += 6;
    }
}

void ptConvert_UYVYToRGBA8888(const bbU8* pSrcY,
                              bbU8* pDst,
                              bbU32 width,
                              const bbS16* pYUV2RGB)
{
    // converts 1 line, dst stride is width*4
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const u  = ((int)pSrcY[0] + pYUV2RGB[1]);
        int const y0 = ((int)pSrcY[1] + pYUV2RGB[0]);
        int const v  = ((int)pSrcY[2] + pYUV2RGB[2]);
        int const y1 = ((int)pSrcY[3] + pYUV2RGB[0]);
        pSrcY += 4;

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4] = p; // R1
        pDst[3] = pDst[4+3] = 255;
        pDst += 8;
    }
}

void ptConvert_YUV422PToRGB888(const bbU8* pSrcY,
                               const bbU8* pSrcU,
                               const bbU8* pSrcV,
                               bbU8* pDst,
                               bbU32 width,
                               const bbS16* pYUV2RGB)
{
    // convert 1 line
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const y0 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
        int const y1 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
        int const u  = ((int)*(pSrcU++) + (int)pYUV2RGB[1]);
        int const v  = ((int)*(pSrcV++) + (int)pYUV2RGB[2]);

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[3] = p; // R1
        pDst += 6;
    }
}

void ptConvert_YUV422PToRGBA8888(const bbU8* pSrcY,
                                 const bbU8* pSrcU,
                                 const bbU8* pSrcV,
                                 bbU8* pDst,
                                 bbU32 width,
                                 const bbS16* pYUV2RGB)
{
    // convert 1 line, dst stride is width*4
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const y0 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
        int const y1 = ((int)*(pSrcY++) + (int)pYUV2RGB[0]);
        int const u  = ((int)*(pSrcU++) + (int)pYUV2RGB[1]);
        int const v  = ((int)*(pSrcV++) + (int)pYUV2RGB[2]);

        int tmp = u * pYUV2RGB[10] + v * pYUV2RGB[11];
        register int p;
        if ((p = (y0 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B0
        if ((p = (y1 * pYUV2RGB[9] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+2] = p; // B1
        tmp = u * pYUV2RGB[7] + v * pYUV2RGB[8];
        if ((p = (y0 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G0
        if ((p = (y1 * pYUV2RGB[6] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4+1] = p; // G1
        tmp = u * pYUV2RGB[4] + v * pYUV2RGB[5];
        if ((p = (y0 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R0
        if ((p = (y1 * pYUV2RGB[3] + tmp)>>10) < 0) p=0;
        if (p>255) p=255;
        pDst[4] = p; // R1
        pDst[3] = pDst[4+3] = 255;
        pDst += 8;
    }
}

void ptConvert_YUV422RPToRGB888(const bbU8* pSrcY0,
                                const bbU8* pSrcY1,
                                const bbU8* pSrcU,
                                const bbU8* pSrcV,
                                bbU8* pDst,
                                bbU8* pDst2,
                                bbU32 width,
                                const bbS16* pYUV2RGB)
{
    // converts 2 lines
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<1) + width;

        while (pDst < pDstEnd)
        {
            int const y = ((int)*(pSrcY++) + pYUV2RGB[0]);
            int const u = ((int)*(pSrcU++) + pYUV2RGB[1]);
            int const v = ((int)*(pSrcV++) + pYUV2RGB[2]);

            register int p;
            if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B
            if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G
            if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R
            pDst += 3;
        }

        pSrcU -= width;
        pSrcV -= width;
        pSrcY = pSrcY1;
        pDst = pDst2;

    } while (pSrcY && --i);
}

void ptConvert_YUV422RPToRGBA8888(const bbU8* pSrcY0,
                                  const bbU8* pSrcY1,
                                  const bbU8* pSrcU,
                                  const bbU8* pSrcV,
                                  bbU8* pDst,
                                  bbU32 width,
                                  const bbS16* pYUV2RGB)
{
    // converts 2 lines, dst stride is width*4
    const bbU8* pSrcY = pSrcY0;
    bbUINT i=2;
    do
    {
        bbU8* const pDstEnd = pDst + (width<<2);

        while (pDst < pDstEnd)
        {
            int const y = ((int)*(pSrcY++) + pYUV2RGB[0]);
            int const u = ((int)*(pSrcU++) + pYUV2RGB[1]);
            int const v = ((int)*(pSrcV++) + pYUV2RGB[2]);

            register int p;
            if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[2] = p; // B
            if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[1] = p; // G
            if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
            if (p>255) p=255;
            pDst[0] = p; // R
            pDst[3] = 255;
            pDst += 4;
        }

        pSrcU -= width;
        pSrcV -= width;
        pSrcY = pSrcY1;

    } while (pSrcY && --i);
}

void ptConvert_YUV444PToRGB888(const bbU8* pSrcY,
                               const bbU8* pSrcU,
                               const bbU8* pSrcV,
                               bbU8* pDst,
                               bbU32 width,
                               const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const y = ((int)*(pSrcY++) + pYUV2RGB[0]);
        int const u = ((int)*(pSrcU++) + pYUV2RGB[1]);
        int const v = ((int)*(pSrcV++) + pYUV2RGB[2]);

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst += 3;
    }
}

void ptConvert_YUV444PToRGBA8888(const bbU8* pSrcY,
                                 const bbU8* pSrcU,
                                 const bbU8* pSrcV,
                                 bbU8* pDst,
                                 bbU32 width,
                                 const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const y = ((int)*(pSrcY++) + pYUV2RGB[0]);
        int const u = ((int)*(pSrcU++) + pYUV2RGB[1]);
        int const v = ((int)*(pSrcV++) + pYUV2RGB[2]);

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst[3] = 255;
        pDst += 4;
    }
}

void ptConvert_YUV444ToRGB888(const bbU8* pSrc,
                              bbU8* pDst,
                              bbU32 width,
                              const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const y = ((int)*(pSrc++) + pYUV2RGB[0]);
        int const u = ((int)*(pSrc++) + pYUV2RGB[1]);
        int const v = ((int)*(pSrc++) + pYUV2RGB[2]);

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst += 3;
    }
}

void ptConvert_YUV444ToRGBA8888(const bbU8* pSrc,
                                bbU8* pDst,
                                bbU32 width,
                                const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const y = ((int)*(pSrc++) + pYUV2RGB[0]);
        int const u = ((int)*(pSrc++) + pYUV2RGB[1]);
        int const v = ((int)*(pSrc++) + pYUV2RGB[2]);

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst[3] = 255;
        pDst += 4;
    }
}

void ptConvert_AYUVToRGB888(const bbU8* pSrc,
                            bbU8* pDst,
                            bbU32 width,
                            const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    while (pDst < pDstEnd)
    {
        int const y = ((int)pSrc[2] + pYUV2RGB[0]);
        int const u = ((int)pSrc[1] + pYUV2RGB[1]);
        int const v = ((int)pSrc[0] + pYUV2RGB[2]); pSrc+=4;

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst += 3;
    }
}

void ptConvert_AYUVToRGBA8888(const bbU8* pSrc,
                              bbU8* pDst,
                              bbU32 width,
                              const bbS16* pYUV2RGB)
{
    bbU8* const pDstEnd = pDst + (width<<2);

    while (pDst < pDstEnd)
    {
        int const y = ((int)pSrc[2] + pYUV2RGB[0]);
        int const u = ((int)pSrc[1] + pYUV2RGB[1]);
        int const v = ((int)pSrc[0] + pYUV2RGB[2]);

        register int p;
        if ((p = (y * pYUV2RGB[9] + u * pYUV2RGB[10]+ v * pYUV2RGB[11]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // B
        if ((p = (y * pYUV2RGB[6] + u * pYUV2RGB[7] + v * pYUV2RGB[8]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // G
        if ((p = (y * pYUV2RGB[3] + u * pYUV2RGB[4] + v * pYUV2RGB[5]) >> 10) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // R
        pDst[3] = pSrc[3]; pSrc+=4;
        pDst += 4;
    }
}

void ptConvert_RGBA8888ToYUYV(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pRGB2YUV)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        int const r1 = (int)pSrc[0];
        int const g1 = (int)pSrc[1];
        int const b1 = (int)pSrc[2]; pSrc += 4;

        register int p = (r1 * pRGB2YUV[3] + g1 * pRGB2YUV[4] + b1 * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // Y1

        int r2 = (int)pSrc[0];
        int g2 = (int)pSrc[1];
        int b2 = (int)pSrc[2]; pSrc += 4;

        p = (r2 * pRGB2YUV[3] + g2 * pRGB2YUV[4] + b2 * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // Y2

        r2 += r1;
        g2 += g1;
        b2 += b1;

        p = (r2 * pRGB2YUV[6] + g2 * pRGB2YUV[7] + b2 * pRGB2YUV[8]) >> 11;
        if ((p+=(int)pRGB2YUV[1]) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // U

        p = (r2 * pRGB2YUV[9] + g2 * pRGB2YUV[10]+ b2 * pRGB2YUV[11]) >> 11;
        if ((p+=(int)pRGB2YUV[2]) < 0) p=0;
        if (p>255) p=255;
        pDst[3] = p; // V

        pDst+=4;
    }
}

void ptConvert_RGBA8888ToUYVY(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pRGB2YUV)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        int const r1 = (int)pSrc[0];
        int const g1 = (int)pSrc[1];
        int const b1 = (int)pSrc[2]; pSrc += 4;

        register int p = (r1 * pRGB2YUV[3] + g1 * pRGB2YUV[4] + b1 * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // Y1

        int r2 = (int)pSrc[0];
        int g2 = (int)pSrc[1];
        int b2 = (int)pSrc[2]; pSrc += 4;

        p = (r2 * pRGB2YUV[3] + g2 * pRGB2YUV[4] + b2 * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        pDst[3] = p; // Y2

        r2 += r1;
        g2 += g1;
        b2 += b1;

        p = (r2 * pRGB2YUV[6] + g2 * pRGB2YUV[7] + b2 * pRGB2YUV[8]) >> 11;
        if ((p+=(int)pRGB2YUV[1]) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // U

        p = (r2 * pRGB2YUV[9] + g2 * pRGB2YUV[10]+ b2 * pRGB2YUV[11]) >> 11;
        if ((p+=(int)pRGB2YUV[2]) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // V

        pDst+=4;
    }
}

void ptConvert_RGBA8888ToYUV444(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pRGB2YUV)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;
    while (pDst < pDstEnd)
    {
        int const r = (int)pSrc[0];
        int const g = (int)pSrc[1];
        int const b = (int)pSrc[2]; pSrc += 4;

        register int p = (r * pRGB2YUV[3] + g * pRGB2YUV[4] + b * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        *pDst++ = p; // Y

        p = (r * pRGB2YUV[6] + g * pRGB2YUV[7] + b * pRGB2YUV[8]) >> 10;
        if ((p+=(int)pRGB2YUV[1]) < 0) p=0;
        if (p>255) p=255;
        *pDst++ = p; // U

        p = (r * pRGB2YUV[9] + g * pRGB2YUV[10]+ b * pRGB2YUV[11]) >> 10;
        if ((p+=(int)pRGB2YUV[2]) < 0) p=0;
        if (p>255) p=255;
        *pDst++ = p; // V
    }
}

void ptConvert_RGBA8888ToAYUV(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pRGB2YUV)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        int const r = (int)pSrc[0];
        int const g = (int)pSrc[1];
        int const b = (int)pSrc[2];

        register int p = (r * pRGB2YUV[3] + g * pRGB2YUV[4] + b * pRGB2YUV[5]) >> 10;
        if ((p+=(int)pRGB2YUV[0]) < 0) p=0;
        if (p>255) p=255;
        pDst[2] = p; // Y

        p = (r * pRGB2YUV[6] + g * pRGB2YUV[7] + b * pRGB2YUV[8]) >> 10;
        if ((p+=(int)pRGB2YUV[1]) < 0) p=0;
        if (p>255) p=255;
        pDst[1] = p; // U

        p = (r * pRGB2YUV[9] + g * pRGB2YUV[10]+ b * pRGB2YUV[11]) >> 10;
        if ((p+=(int)pRGB2YUV[2]) < 0) p=0;
        if (p>255) p=255;
        pDst[0] = p; // V

        pDst[3] = pSrc[3];
        pSrc += 4;
        pDst += 4;
    }
}

void ptConvert_RGBA8888ToRGB565(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        register bbU32 rgb = (bbU32)(*pSrc++) >> 3;
        register bbU32 tmp = (bbU32)(*pSrc++) & 0xFCU; rgb |= tmp << (5-2);
        tmp = (bbU32)*pSrc & 0xF8U; rgb |= tmp << (5+6-3); pSrc+=2;
        bbST16LE(pDst, rgb); pDst+=2;
    }
}

void ptConvert_RGBA8888ToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;
    while (pDst < pDstEnd)
    {
        register bbU32 rgba = bbLD32LE(pSrc); pSrc+=4;
        bbST24LE(pDst, rgba); pDst+=3;
    }
}

void ptConvert_RGBA8888ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;
    while (pDst < pDstEnd)
    {
        register bbU32 rgba = bbLD32LE(pSrc); pSrc+=4;
        *pDst++ = (bbU8)(rgba>>16);
        *pDst++ = (bbU8)(rgba>>8);
        *pDst++ = (bbU8)rgba;
    }
}

void ptConvert_RGBA8888ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    bbU32 const mask = 0xFF00FF00UL;
    while (pDst < pDstEnd)
    {
        register bbU32 rgba = bbLD32LE(pSrc); pSrc+=4;
        register bbU32 gb = rgba &~ mask;
        rgba = (rgba & mask) | ((gb>>16) | (gb<<16));
        bbST32LE(pDst, rgba); pDst+=4;
    }
}

void ptConvert_RGB565ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;
    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgb = bbLD16LE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgb >> (11-3); *pDst++ = c & (0x1F<<3);
            c = rgb >> (5-2); *pDst++ = c & (0x3F<<2);
            *pDst++ = (bbU8)(rgb<<3);
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgb = bbLD16BE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgb >> (11-3); *pDst++ = c & (0x1F<<3);
            c = rgb >> (5-2); *pDst++ = c & (0x3F<<2);
            *pDst++ = (bbU8)(rgb<<3);
        }
    }
}

void ptConvert_RGB565ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgb = bbLD16LE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgb >> (11-3); *pDst++ = c & (0x1F<<3);
            c = rgb >> (5-2); *pDst++ = c & (0x3F<<2);
            *pDst++ = (bbU8)(rgb<<3);
            *pDst++ = 255;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgb = bbLD16BE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgb >> (11-3); *pDst++ = c & (0x1F<<3);
            c = rgb >> (5-2); *pDst++ = c & (0x3F<<2);
            *pDst++ = (bbU8)(rgb<<3);
            *pDst++ = 255;
        }
    }
}

void ptConvert_RGB565ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        register bbU32 rgb = bbLD16LE(pSrc); pSrc+=2;
        register bbU32 c;
        *pDst++ = (bbU8)(rgb<<3);
        c = rgb >> (5-2); *pDst++ = c & (0x3F<<2);
        c = rgb >> (11-3); *pDst++ = c & (0x1F<<3);
        *pDst++ = 255;
    }
}

void ptConvert_RGBA1555ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbS32 rgba = bbLD16LE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgba >> (10-3); *pDst++ = c & (0x1F<<3);
            c = rgba >> (5-3); *pDst++ = c & (0x1F<<3);
            *pDst++ = (bbU8)(rgba<<3);
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbS32 rgba = bbLD16BE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgba >> (10-3); *pDst++ = c & (0x1F<<3);
            c = rgba >> (5-3); *pDst++ = c & (0x1F<<3);
            *pDst++ = (bbU8)(rgba<<3);
        }
    }
}

void ptConvert_RGBA1555ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<2);

    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbS32 rgba = bbLDS16LE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgba >> (10-3); *pDst++ = c & (0x1F<<3);
            c = rgba >> (5-3); *pDst++ = c & (0x1F<<3);
            *pDst++ = (bbU8)(rgba<<3);
            *pDst++ = rgba >> 24;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbS32 rgba = bbLDS16BE(pSrc); pSrc+=2;
            register bbU32 c;
            c = rgba >> (10-3); *pDst++ = c & (0x1F<<3);
            c = rgba >> (5-3); *pDst++ = c & (0x1F<<3);
            *pDst++ = (bbU8)(rgba<<3);
            *pDst++ = rgba >> 24;
        }
    }
}

void ptConvert_RGBA1555ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        register bbS32 rgba = bbLDS16LE(pSrc); pSrc+=2;
        register bbU32 c;
        *pDst++ = (bbU8)(rgba<<3);
        c = rgba >> (5-3); *pDst++ = c & (0x1F<<3);
        c = rgba >> (10-3); *pDst++ = c & (0x1F<<3);
        *pDst++ = rgba >> 24;
    }
}

void ptConvert_RGBA4444ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;

    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgba = bbLD16LE(pSrc); pSrc+=2;
            *pDst++ = (bbU8)((rgba>>4) & 0xF0);
            *pDst++ = (bbU8)(rgba & 0xF0);
            *pDst++ = (bbU8)(rgba<<4);
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgba = bbLD16BE(pSrc); pSrc+=2;
            *pDst++ = (bbU8)((rgba>>4) & 0xF0);
            *pDst++ = (bbU8)(rgba & 0xF0);
            *pDst++ = (bbU8)(rgba<<4);
        }
    }
}

void ptConvert_RGBA4444ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<2);

    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgba = bbLD16LE(pSrc); pSrc+=2;
            *pDst++ = (bbU8)((rgba>>4) & 0xF0);
            *pDst++ = (bbU8)(rgba & 0xF0);
            *pDst++ = (bbU8)(rgba<<4);
            register bbU32 a = rgba>>12;
            *pDst++ = (bbU8)(a|(a<<4));
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            register bbU32 rgba = bbLD16BE(pSrc); pSrc+=2;
            *pDst++ = (bbU8)((rgba>>4) & 0xF0);
            *pDst++ = (bbU8)(rgba & 0xF0);
            *pDst++ = (bbU8)(rgba<<4);
            register bbU32 a = rgba>>12;
            *pDst++ = (bbU8)(a|(a<<4));
        }
    }
}

void ptConvert_RGBA4444ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        register bbU32 rgba = bbLD16LE(pSrc); pSrc+=2;
        *pDst++ = (bbU8)(rgba<<4);
        *pDst++ = (bbU8)(rgba & 0xF0);
        *pDst++ = (bbU8)((rgba>>4) & 0xF0);
        register bbU32 a = rgba>>12;
        *pDst++ = (bbU8)(a|(a<<4));
    }
}

void ptConvert_RGB888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        register bbU32 rgb = bbLD24LE(pSrc); pSrc+=3;
        rgb |= 0xFF000000UL;
        bbST32LE(pDst, rgb); pDst += 4;
    }
}

void ptConvert_BGR888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    while (pDst < pDstEnd)
    {
        pDst[2] = *pSrc++; //b
        pDst[1] = *pSrc++; //g
        pDst[0] = *pSrc++; //r
        pDst[3] = 255;
        pDst+=4;
    }
}

void ptConvert_BGR888ToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<1) + width;
    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2] = *pSrc++; //b
            pDst[1] = *pSrc++; //g
            pDst[0] = *pSrc++; //r
            pDst+=3;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[0] = *pSrc++; //b
            pDst[1] = *pSrc++; //g
            pDst[2] = *pSrc++; //r
            pDst+=3;
        }
    }
}

void ptConvert_BGRA8888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2] = *pSrc++; //b
            pDst[1] = *pSrc++; //g
            pDst[0] = *pSrc++; //r
            pDst[3] = *pSrc++; //a
            pDst+=4;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[3] = *pSrc++; //a
            pDst[0] = *pSrc++; //r
            pDst[1] = *pSrc++; //g
            pDst[2] = *pSrc++; //b
            pDst+=4;
        }
    }
}

void ptConvert_RGB888PToRGBA8888(const bbU8* pR, const bbU8* pG, const bbU8* pB, bbU8* pDst, bbU32 width, ptENDIAN endian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (endian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[0] = *pR++; //r
            pDst[1] = *pG++; //g
            pDst[2] = *pB++; //b
            pDst[3] = 255;   //a
            pDst+=4;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[0] = 255;   //a
            pDst[1] = *pB++; //b
            pDst[2] = *pG++; //g
            pDst[3] = *pR++; //r
            pDst+=4;
        }
    }
}

void ptConvert_RGBA8888ToRGB888P(const bbU8* pSrc, bbU8* pDstR, bbU8* pDstG, bbU8* pDstB, bbU32 width)
{
    bbU8* const pDstREnd = pDstR + width;
    while (pDstR < pDstREnd)
    {
        *pDstB++ = (int)pSrc[2];
        *pDstG++ = (int)pSrc[1];
        *pDstR++ = (int)pSrc[0]; pSrc += 4;
    }
}

void ptConvert_YUYVToAYUV(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN dstEndian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (dstEndian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2]   = pSrc[0];             // y0
            pDst[4+2] = pSrc[2];             // y1
            pDst[1]   = pDst[4+1] = pSrc[1]; // u
            pDst[0]   = pDst[4+0] = pSrc[3]; // v
            pDst[3]   = pDst[4+3] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[1]   = pSrc[3];             // y0
            pDst[4+1] = pSrc[1];             // y1
            pDst[2]   = pDst[4+2] = pSrc[1]; // u
            pDst[3]   = pDst[4+3] = pSrc[3]; // v
            pDst[0]   = pDst[4+0] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
}

void ptConvert_YVYUToAYUV(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN dstEndian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (dstEndian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2]   = pSrc[0];             // y0
            pDst[4+2] = pSrc[2];             // y1
            pDst[1]   = pDst[4+1] = pSrc[3]; // u
            pDst[0]   = pDst[4+0] = pSrc[1]; // v
            pDst[3]   = pDst[4+3] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[1]   = pSrc[3];             // y0
            pDst[4+1] = pSrc[1];             // y1
            pDst[2]   = pDst[4+2] = pSrc[3]; // u
            pDst[3]   = pDst[4+3] = pSrc[1]; // v
            pDst[0]   = pDst[4+0] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
}

void ptConvert_UYVYToAYUV(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN dstEndian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (dstEndian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2]   = pSrc[1];             // y0
            pDst[4+2] = pSrc[3];             // y1
            pDst[1]   = pDst[4+1] = pSrc[0]; // u
            pDst[0]   = pDst[4+0] = pSrc[2]; // v
            pDst[3]   = pDst[4+3] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[1]   = pSrc[1];             // y0
            pDst[4+1] = pSrc[3];             // y1
            pDst[2]   = pDst[4+2] = pSrc[0]; // u
            pDst[3]   = pDst[4+3] = pSrc[2]; // v
            pDst[0]   = pDst[4+0] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
}

void ptConvert_VYUYToAYUV(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN dstEndian)
{
    bbU8* const pDstEnd = pDst + (width<<2);
    if (dstEndian == ptENDIAN_LE)
    {
        while (pDst < pDstEnd)
        {
            pDst[2]   = pSrc[1];             // y0
            pDst[4+2] = pSrc[3];             // y1
            pDst[1]   = pDst[4+1] = pSrc[2]; // u
            pDst[0]   = pDst[4+0] = pSrc[0]; // v
            pDst[3]   = pDst[4+3] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
    else
    {
        while (pDst < pDstEnd)
        {
            pDst[1]   = pSrc[1];             // y0
            pDst[4+1] = pSrc[3];             // y1
            pDst[2]   = pDst[4+2] = pSrc[2]; // u
            pDst[3]   = pDst[4+3] = pSrc[0]; // v
            pDst[0]   = pDst[4+0] = 255;     // a
            pSrc += 4;
            pDst += 8;
        }
    }
}

void ptConvert_AYUVToYUYV(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        pDst[0] = pSrc[2];   // y0
        pDst[2] = pSrc[4+2]; // y1
        pDst[1] = ((int)pSrc[1] + (int)pSrc[4+1])>>1; // u
        pDst[3] = ((int)pSrc[0] + (int)pSrc[4+0])>>1; // v
        pSrc += 8;
        pDst += 4;
    }
}

void ptConvert_AYUVToYVYU(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        pDst[0] = pSrc[2];   // y0
        pDst[2] = pSrc[4+2]; // y1
        pDst[1] = ((int)pSrc[0] + (int)pSrc[4+0])>>1; // v
        pDst[3] = ((int)pSrc[1] + (int)pSrc[4+1])>>1; // u
        pSrc += 8;
        pDst += 4;
    }
}

void ptConvert_AYUVToUYVY(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        pDst[1] = pSrc[2];   // y0
        pDst[3] = pSrc[4+2]; // y1
        pDst[0] = ((int)pSrc[1] + (int)pSrc[4+1])>>1; // u
        pDst[2] = ((int)pSrc[0] + (int)pSrc[4+0])>>1; // v
        pSrc += 8;
        pDst += 4;
    }
}

void ptConvert_AYUVToVYUY(const bbU8* pSrc, bbU8* pDst, bbU32 width)
{
    bbU8* const pDstEnd = pDst + (width<<1);
    while (pDst < pDstEnd)
    {
        pDst[1] = pSrc[2];   // y0
        pDst[3] = pSrc[4+2]; // y1
        pDst[0] = ((int)pSrc[0] + (int)pSrc[4+0])>>1; // v
        pDst[2] = ((int)pSrc[1] + (int)pSrc[4+1])>>1; // u
        pSrc += 8;
        pDst += 4;
    }
}

