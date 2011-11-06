#include "ptconvert.h"

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
        bbST32LE(pDst, b); pDst+=4;
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
            bbST16LE(pDst, b); pDst+=2;
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
            *pDst = pColIdxLU[*pSrc];
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

void ptConvert_AYUVToRGBA8888(const bbU8* pSrc,
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
        pDst[3] = *pSrc++;
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
            register bbU32 a = rgba>>12;
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

