#include "babel/mem.h"
#include "ptSprite.h"
#include "ptPalMan.h"
#include "ptconvert.h"

void ptSprite::Destroy()
{
}

void ptSprite::Clear()
{
    Destroy();
    bbMemClear(this, sizeof(*this));
}

void ptSprite::Create(bbU8** pPlanes, bbU32 width, bbU32 height, bbU32 stride, bbU32 strideUV, ptCOLFMT fmt, ptENDIAN endian, ptBITORDER bitorder)
{
    Clear();

    bbUINT planes = ptgColFmtInfo[fmt].PlaneCount;

    switch(fmt)
    {
    case ptCOLFMT_YUV420P:
    case ptCOLFMT_YUV420P_YV12:
    case ptCOLFMT_YUV420P_IMC3:
    case ptCOLFMT_YUV420P_IMC1:
    case ptCOLFMT_YUV420P_12:
    case ptCOLFMT_YUV420P_16:
        this->pPlane[0] = pPlanes[0];
        this->pPlane[1] = pPlanes[0] + stride;
        this->pPlane[2] = pPlanes[1];
        this->pPlane[3] = pPlanes[2];
        break;
    case ptCOLFMT_YUV420P_IMC4:
    case ptCOLFMT_YUV420P_IMC2:
        this->pPlane[0] = pPlanes[0];
        this->pPlane[1] = pPlanes[0] + stride;
        this->pPlane[2] = pPlanes[1];
        this->pPlane[3] = pPlanes[1] + (strideUV>>1);
        bbASSERT((strideUV&1)==0);
        break;
    case ptCOLFMT_YUV420P_NV12:
    case ptCOLFMT_YUV420P_NV21:
        this->pPlane[0] = pPlanes[0];
        this->pPlane[1] = pPlanes[0] + stride;
        this->pPlane[2] = pPlanes[1];
        break;
    default:
        for(bbUINT i=0; i<planes; i++)
            this->pPlane[i] = pPlanes[i];
        break;
    }

    this->width = width;
    this->height = height;
    this->stride = stride;
    this->strideUV = strideUV;
    this->colfmt = fmt;
    this->endian = endian;
    this->bitorder = bitorder;
}

void ptSprite::Create(bbU8* pData, bbU32 width, bbU32 height, bbU32 stride, ptCOLFMT fmt, ptENDIAN endian, ptBITORDER bitorder)
{
    Create(&pData, width, height, stride, 0, fmt, endian, bitorder);
}

static void ptMakePal2PalLookup(const ptPal* pSrc, const ptPal* pDst, bbU8* pLU)
{
    bbUINT srcColCount = pSrc->GetColCount();

    for(bbUINT i=0; i<srcColCount; i++)
    {
        ptRGBA srcCol = pSrc->GetColRGBA(i);
        bbUINT dstBestIdx = pDst->MatchRGBA(srcCol);
        *pLU++ = (bbU8)dstBestIdx;
    }
}

bbERR ptSprite::Convert_Pal2Pal(ptSprite* pDst) const
{
    const bbU8* pDataSrc;
    bbU8*       pDataDst;
    const bbU8* pLineBuf;
    bbU32       lineBufWidth;
    bbU32       width;
    bbUINT      plane;
    const bbU8* pLineSrc = this->GetData();
    bbU8*       pLineDst = pDst->GetData();
    bbU32       height   = this->height;
    bbU8        colIdxLU[256];
    bbU8        linebuf[256];

    if (GetPal()->GetColCount() > sizeof(colIdxLU))
        return bbErrSet(bbEBADPARAM);

    ptMakePal2PalLookup(this->GetPal(), pDst->GetPal(), colIdxLU);

    while (height-->0)
    {
        pDataSrc = pLineSrc; pLineSrc += this->GetStride();
        pDataDst = pLineDst; pLineDst += pDst->GetStride();

        width = this->GetWidth();
        do
        {
            pLineBuf = linebuf;
            lineBufWidth = width>sizeof(linebuf) ? sizeof(linebuf) : width;
                
            // - convert source format to 8bpp
            switch(this->GetColFmt())
            {
            case ptCOLFMT_1BPP:
                ptExpand_1BppTo8Bpp(pDataSrc, linebuf, lineBufWidth, this->GetBitOrder());
                pDataSrc += sizeof(linebuf)>>3;
                break;
            case ptCOLFMT_2BPP:
                ptExpand_2BppTo8Bpp(pDataSrc, linebuf, lineBufWidth, this->GetBitOrder());
                pDataSrc += sizeof(linebuf)>>2;
                break;
            case ptCOLFMT_4BPP:
                ptExpand_4BppTo8Bpp(pDataSrc, linebuf, lineBufWidth, this->GetBitOrder());
                pDataSrc += sizeof(linebuf)>>1;
                break;
            case ptCOLFMT_8BPP:
                pLineBuf = pDataSrc;
                pDataSrc += sizeof(linebuf);
                break;
            case ptCOLFMT_2BPPP:
            case ptCOLFMT_3BPPP:
            case ptCOLFMT_4BPPP:
            case ptCOLFMT_5BPPP:
            case ptCOLFMT_6BPPP:
            case ptCOLFMT_7BPPP:
            case ptCOLFMT_8BPPP:
                ptExpand_1BppTo8Bpp(pDataSrc, linebuf, lineBufWidth, this->GetBitOrder());
                for(plane=1; plane < (bbUINT)(GetColFmt()-ptCOLFMT_2BPPP+2); plane++)
                    ptMerge_1BppTo8Bpp(this->GetPlane(plane) + (bbU32)pDataSrc-(bbU32)this->GetData(), linebuf, lineBufWidth, this->GetBitOrder(), plane);
                pDataSrc += sizeof(linebuf)>>3;
                break;
            default: return bbErrSet(bbENOTSUP);
            }

            // - convert 8bpp to target format
            switch(pDst->GetColFmt())
            {
            case ptCOLFMT_4BPP:
                if (pDst->GetBitOrder() == ptBITORDER_LSBLEFT)
                    ptConvert_8BppTo4BppLSB(pLineBuf, pDataDst, lineBufWidth, colIdxLU);
                else
                    ptConvert_8BppTo4BppMSB(pLineBuf, pDataDst, lineBufWidth, colIdxLU);
                pDataDst += sizeof(linebuf)>>1;
                break;
            case ptCOLFMT_8BPP:
                ptConvert_8BppTo8Bpp(pLineBuf, pDataDst, lineBufWidth, colIdxLU);
                pDataDst += sizeof(linebuf);
                break;
            default: return bbErrSet(bbENOTSUP);
            }

            width -= sizeof(linebuf);

        } while ((int)width > 0);
    }

    return bbEOK;
}

bbERR ptSprite::Convert_YUV2Pal(ptSprite* /*pDst*/) const
{
    return bbErrSet(bbENOTSUP);
}

bbERR ptSprite::Convert_YUV2RGB(ptSprite* pDst) const
{
    bbU32        offsetY  = 0;
    bbU32        offsetUV = 0;
    bbU8*        pDataDst = pDst->GetData();
    bbU8*        pDataTmp;
    bbU8*        pLineBuf = NULL;
    const bbS16* pYUV2RGB;
    bbUINT       lines    = 1;
    bbU32        height   = this->height;
    bbU32        heightEnd;
    ptYUV2RGB    yuv2rgb_swapUV;

    if (pDst->GetColFmt() != ptCOLFMT_RGBA8888)
    {
        if (NULL == (pLineBuf = (bbU8*)bbMemAlloc(this->width * 4 * 2)))
            return bbELAST;
    }

    if (ptgColFmtInfo[this->colfmt].flags & ptCOLFMTFLAG_SWAPUV) // VU order?
    {
        yuv2rgb_swapUV = *this->poYUV2RGB;
        yuv2rgb_swapUV.SwapUV();
        pYUV2RGB = yuv2rgb_swapUV.GetMatrix();
    }
    else
    {
        pYUV2RGB = this->poYUV2RGB->GetMatrix();
    }

    while (height>0)
    {
        if (pDst->GetColFmt() == ptCOLFMT_RGBA8888)
            pDataTmp = pDataDst;
        else
            pDataTmp = pLineBuf;

        // - convert 2 lines of source YUV to ARGB8888
        switch(this->GetColFmt())
        {
        case ptCOLFMT_YUV420P:
        case ptCOLFMT_YUV420P_YV12:
        case ptCOLFMT_YUV420P_IMC3:
        case ptCOLFMT_YUV420P_IMC1:
        case ptCOLFMT_YUV420P_IMC4:
        case ptCOLFMT_YUV420P_IMC2:
            ptConvert_YUV420ToRGBA8888(this->pPlane[0] + offsetY,
                                       (height==1) ? NULL : this->pPlane[1] + offsetY,
                                       this->pPlane[2] + offsetUV,
                                       this->pPlane[3] + offsetUV,
                                       pDataTmp,
                                       this->width,
                                       pYUV2RGB);
            offsetY  += this->GetStride()<<1;
            offsetUV += this->GetStrideUV();
            lines = 2;
            break;

        case ptCOLFMT_YUV420P_NV12:
        case ptCOLFMT_YUV420P_NV21:
            ptConvert_YUVNV12ToRGBA8888(this->pPlane[0] + offsetY,
                                        (height==1) ? NULL : this->pPlane[1] + offsetY,
                                        this->pPlane[2] + offsetUV,
                                        pDataTmp,
                                        this->width,
                                        pYUV2RGB);
            offsetY  += this->GetStride()<<1;
            offsetUV += this->GetStrideUV();
            lines = 2;
            break;

        case ptCOLFMT_YUV420P_12:
        case ptCOLFMT_YUV420P_16:
            if (this->GetEndian() == ptENDIAN_LE)
                ptConvert_YUV42016LEToRGBA8888(this->pPlane[0] + offsetY,
                                               (height==1) ? NULL : this->pPlane[1] + offsetY,
                                               this->pPlane[2] + offsetUV,
                                               this->pPlane[3] + offsetUV,
                                               pDataTmp,
                                               this->width,
                                               pYUV2RGB,
                                               this->GetColFmt()==ptCOLFMT_YUV420P_12 ? 4 : 8);
            else
                ptConvert_YUV42016BEToRGBA8888(this->pPlane[0] + offsetY,
                                               (height==1) ? NULL : this->pPlane[1] + offsetY,
                                               this->pPlane[2] + offsetUV,
                                               this->pPlane[3] + offsetUV,
                                               pDataTmp,
                                               this->width,
                                               pYUV2RGB,
                                               this->GetColFmt()==ptCOLFMT_YUV420P_12 ? 4 : 8);
            offsetY  += this->GetStride()<<1;
            offsetUV += this->GetStrideUV();
            lines = 2;
            break;

        case ptCOLFMT_YUYV:
        case ptCOLFMT_YVYU:
            ptConvert_YUYVToRGBA8888(this->pPlane[0] + offsetY,
                                     pDataTmp,
                                     this->width,
                                     pYUV2RGB);
            offsetY  += this->GetStride();
            break;

        case ptCOLFMT_UYVY:
        case ptCOLFMT_VYUY:
            ptConvert_UYVYToRGBA8888(this->pPlane[0] + offsetY,
                                     pDataTmp,
                                     this->width,
                                     pYUV2RGB);
            offsetY  += this->GetStride();
            break;

        case ptCOLFMT_YUV422P:
            ptConvert_YUV422PToRGBA8888(this->pPlane[0] + offsetY,
                                        this->pPlane[1] + offsetUV,
                                        this->pPlane[2] + offsetUV,
                                        pDataTmp,
                                        this->width,
                                        pYUV2RGB);
            offsetY  += this->GetStride();
            offsetUV += this->GetStrideUV();
            break;

        case ptCOLFMT_YUV422RP:
            ptConvert_YUV422RPToRGBA8888(this->pPlane[0] + offsetY,
                                         (height==1) ? NULL : this->pPlane[0] + offsetY + this->GetStride(),
                                         this->pPlane[1] + offsetUV,
                                         this->pPlane[2] + offsetUV,
                                         pDataTmp,
                                         this->width,
                                         pYUV2RGB);
            offsetY  += this->GetStride()<<1;
            offsetUV += this->GetStrideUV();
            lines = 2;
            break;

        case ptCOLFMT_YUV444:
            ptConvert_YUV444ToRGBA8888(this->pPlane[0] + offsetY,
                                       pDataTmp,
                                       this->width,
                                       pYUV2RGB);
            offsetY += this->GetStride();
            break;

        case ptCOLFMT_YUV444P:
            ptConvert_YUV444PToRGBA8888(this->pPlane[0] + offsetY,
                                        this->pPlane[1] + offsetUV,
                                        this->pPlane[2] + offsetUV,
                                        pDataTmp,
                                        this->width,
                                        pYUV2RGB);
            offsetY  += this->GetStride();
            offsetUV += this->GetStrideUV();
            break;

        case ptCOLFMT_AYUV:
            ptConvert_AYUVToRGBA8888(this->pPlane[0] + offsetY,
                                     pDataTmp,
                                     this->width,
                                     pYUV2RGB);
            offsetY += this->GetStride();
            break;

        default:
            bbErrSet(bbENOTSUP);
            goto ptSprite_Convert_YUV2RGB_err;
        }

        // - convert N lines of ARGB8888 to target RGB
        heightEnd = height>=lines ? height-lines : height-1;
        do
        {
            switch(pDst->GetColFmt())
            {
            case ptCOLFMT_RGB565: ptConvert_RGBA8888ToRGB565(pDataTmp, pDataDst, this->width); break;
            case ptCOLFMT_RGB888: ptConvert_RGBA8888ToRGB888(pDataTmp, pDataDst, this->width); break;
            case ptCOLFMT_BGR888: ptConvert_RGBA8888ToBGR888(pDataTmp, pDataDst, this->width); break;
            case ptCOLFMT_RGBA8888: break;
            case ptCOLFMT_BGRA8888: ptConvert_RGBA8888ToBGRA8888(pDataTmp, pDataDst, this->width); break;
            default:
                bbErrSet(bbENOTSUP);
                goto ptSprite_Convert_YUV2RGB_err;
            }
            pDataTmp += (this->width<<2);
            pDataDst += pDst->GetStride();

        } while(--height != heightEnd);
    }

    bbMemFree(pLineBuf);
    return bbEOK;

    ptSprite_Convert_YUV2RGB_err:
    bbMemFree(pLineBuf);
    return bbELAST;
}

bbERR ptSprite::Convert_RGB2RGB(ptSprite* pDst) const
{
    const bbU8* pDataSrc = this->pData;
    bbU8*       pDataDst = pDst->pData; 
    bbU8*       pDataTmp;
    bbU8*       pLineBuf = NULL;
    bbU32       height   = this->height;
    
    if (this->GetColFmt() != ptCOLFMT_RGBA8888)
    {
        if (NULL == (pLineBuf = (bbU8*)bbMemAlloc(this->width * 4)))
            return bbELAST;
    }

    while (height-->0)
    {
        pDataTmp = pLineBuf;

        // - convert 1 line of source RGB to ARGB8888
        switch(this->GetColFmt())
        {
        case ptCOLFMT_RGB565:   ptConvert_RGB565ToRGBA8888(pDataSrc, pDataTmp, this->width); break;
        case ptCOLFMT_RGBA1555: ptConvert_RGBA1555ToRGBA8888(pDataSrc, pDataTmp, this->width); break;
        case ptCOLFMT_RGBA4444: ptConvert_RGBA4444ToRGBA8888(pDataSrc, pDataTmp, this->width); break;
        case ptCOLFMT_RGB888:   ptConvert_RGB888ToRGBA8888(pDataSrc, pDataTmp, this->width); break;
        case ptCOLFMT_BGR888:   ptConvert_BGR888ToRGBA8888(pDataSrc, pDataTmp, this->width); break;
        case ptCOLFMT_RGBA8888: pDataTmp = (bbU8*)pDataSrc; break;
        case ptCOLFMT_BGRA8888: ptConvert_BGRA8888ToRGBA8888(pDataSrc, pDataTmp, this->width, GetEndian()); break;
        default:
            bbErrSet(bbENOTSUP);
            goto ptSprite_Convert_RGB2RGB_err;
        }
        pDataSrc += this->stride;

        // - convert 1 line of ARGB8888 to target RGB
        switch(pDst->GetColFmt())
        {
        case ptCOLFMT_RGB565: ptConvert_RGBA8888ToRGB565(pDataTmp, pDataDst, this->width); break;
        case ptCOLFMT_RGB888: ptConvert_RGBA8888ToRGB888(pDataTmp, pDataDst, this->width); break;
        case ptCOLFMT_BGR888: ptConvert_RGBA8888ToBGR888(pDataTmp, pDataDst, this->width); break;
        case ptCOLFMT_RGBA8888: bbMemMove(pDataDst, pDataTmp, this->width<<2); break;
        case ptCOLFMT_BGRA8888: ptConvert_RGBA8888ToBGRA8888(pDataTmp, pDataDst, this->width); break;
        default:
            bbErrSet(bbENOTSUP);
            goto ptSprite_Convert_RGB2RGB_err;
        }
        pDataDst += pDst->stride;
    }

    bbMemFree(pLineBuf);
    return bbEOK;

    ptSprite_Convert_RGB2RGB_err:
    bbMemFree(pLineBuf);
    return bbELAST;
}

bbERR ptSprite::Convert(ptSprite* pDst) const
{
    if ((this->width != pDst->width) ||
        (this->height != pDst->height))
    {
        return bbErrSet(bbENOTSUP);
    }

    switch (ptColFmtGetType(GetColFmt()))
    {
    case ptCOLTYPE_PAL:
        switch (ptColFmtGetType(pDst->GetColFmt()))
        {
        case ptCOLTYPE_PAL: return Convert_Pal2Pal(pDst);
        default:            return bbErrSet(bbENOTSUP);
        }
        break;
    case ptCOLTYPE_RGB:
        switch (ptColFmtGetType(pDst->GetColFmt()))
        {
        case ptCOLTYPE_RGB: return Convert_RGB2RGB(pDst);
        default:            return bbErrSet(bbENOTSUP);
        }
        break;
    case ptCOLTYPE_YUV:
        switch (ptColFmtGetType(pDst->GetColFmt()))
        {
        case ptCOLTYPE_PAL: return Convert_YUV2Pal(pDst);
        case ptCOLTYPE_RGB: return Convert_YUV2RGB(pDst);
        default:            return bbErrSet(bbENOTSUP);
        }
        break;

    default:
        return bbErrSet(bbENOTSUP);
    }

    return bbEOK;
}

/** Get byte size a rasterline with given width and bitdepth would consume inside ptSprite.
    @param pixelwidth Width in pixels
    @param depth Log2 of colour depth in bits per pixel
    @return Number of bytes (includes possible alignment)
*/
static bbUINT ptSpriteGetBytePitch( const bbUINT pixelwidth, const bbUINT depth)
{
    bbUINT const align = (1 << (5-depth)) - 1;
    bbUINT pitch = (pixelwidth + align) &~ align;
    if (depth > 3) pitch<<=depth-3; else pitch>>=3-depth;
    return pitch;
}

ptSprite* ptSpriteCreate( bbUINT const width, bbUINT const height, bbUINT const depth)
{
    static const bbU8 gDepth2ColFmt[] =
    {
        ptCOLFMT_1BPP,
        ptCOLFMT_2BPP,
        ptCOLFMT_4BPP,
        ptCOLFMT_8BPP,
        ptCOLFMT_RGB565,
        ptCOLFMT_RGBA8888
    };
        
    // allocate memory
    bbUINT const bytepitch = ptSpriteGetBytePitch( width, depth);
    ptSprite* const pSprite = (ptSprite*) bbMemAlloc( ptSIZEOF_SPRITEHDR + (bbU32)((bbU32)bytepitch * height) );
    if (!pSprite) return pSprite;

    pSprite->width   = width;
    pSprite->height  = height;
    pSprite->stride  = bytepitch;
    pSprite->colfmt  = gDepth2ColFmt[depth];
    pSprite->flags   = 0;

    return pSprite;
}

ptSprite* ptSpriteConvert( const ptSprite* const pSrcSprite, bbUINT const depth)
{
    bbUINT      y;
    const bbU8* pSrc;
    bbU8*       pDst;

    // create destination sprite
    ptSprite* const pDstSprite = ptSpriteCreate( pSrcSprite->width, pSrcSprite->height, depth);
    if (!pDstSprite) return pDstSprite;

    // allocate rasterline buffer for 32bpp temp (only, if src is not 32bpp)
    bbU32* pLineBuf = NULL;
    if (pSrcSprite->colfmt != ptCOLFMT_RGBA8888)
    {
        // - align pixel width to 8, because we do not do special checks for unaligned widths
        pLineBuf = (bbU32*) bbMemAlloc(((pSrcSprite->width + 7) &~ 7) << 2);
        if (!pLineBuf) goto ptSpriteConvert_err2;
    }

    // set up pixel data pointers
    pSrc = &pSrcSprite->data[0];
    pDst = &pDstSprite->data[0];

    // loop through raster lines
    for (y = 0; y < pSrcSprite->height; ++y)
    {
        // read a line from the source sprite to 32 bpp

        register bbU32*       pBuf    = pLineBuf;
                 bbU32* const pBufEnd = pBuf + pSrcSprite->width;
        const    bbU32* const pPal    = pSrcSprite->pPal->mpRGB;

        switch (pSrcSprite->colfmt)
        {
        case ptCOLFMT_1BPP:
            {
                register const bbU8* pTmp = pSrc;
                while (pBuf < pBufEnd)
                {
                    bbUINT pix = (bbUINT) *(pTmp++) | 0x100U;
                    do
                    {
                        *(pBuf++) = pPal[ pix & 1U ];
                    } while ((pix >>= 1U) != 1U);
                }
            }
            break;

        case ptCOLFMT_2BPP:
            {
                register const bbU8* pTmp = pSrc;
                while (pBuf < pBufEnd)
                {
                    const bbUINT pix = (bbUINT) *(pTmp++);
                    *(pBuf++) = pPal[ pix & 3U ];
                    *(pBuf++) = pPal[ (pix>>2) & 3U ];
                    *(pBuf++) = pPal[ (pix>>4) & 3U ];
                    *(pBuf++) = pPal[ pix>>6 ];
                }
            }
            break;

        case ptCOLFMT_4BPP:
            {
                register const bbU8* pTmp = pSrc;
                while (pBuf < pBufEnd)
                {
                    const bbUINT pix = (bbUINT) *(pTmp++);
                    *(pBuf++) = pPal[ pix & 15 ];
                    *(pBuf++) = pPal[ pix >> 4 ];
                }
            }
            break;

        case ptCOLFMT_8BPP:
            {
                register const bbU8* pTmp = pSrc;
                while (pBuf < pBufEnd)
                {
                    *(pBuf++) = pPal[ *(pTmp++) ];
                }
            }
            break;

        case ptCOLFMT_RGB565:
            {
                #if pt16BPP == pt16BPP_BGR555
                bbU32 const mask = 0xF800F8UL;
                #endif

                register const bbU16* pTmp = (const bbU16*) pSrc;
                while (pBuf < pBufEnd)
                {
                    register bbU32 rgb16 = (bbU32) *(pTmp++);

                    #if pt16BPP == pt16BPP_BGR555

                    register bbU32 rgb32 = rgb16 | (rgb16 << (9-3));
                    rgb32 = mask & (rgb32<<3);
                    rgb32 &= 31U<<5;
                    rgb16 |= rgb32<<6;
                    //MOV R8,#0xF800F8
                    //ORR R1,R0,R0,LSL #9-3
                    //AND R1,R8,R1,LSL #3
                    //AND R0,R0,#31<<5
                    //ORR R1,R1,R0,LSL #6

                    #elif pt16BPP == pt16BPP_RGB565

                    register bbU32 rgb32 = (rgb16>>11) | (rgb16 << (8-3));
                    rgb32 = mask & (rgb32<<3);
                    rgb32 &= 63U<<5;
                    rgb16 |= rgb32<<5;

                    #endif

                    *(pBuf++) = rgb32;
                }
            }
            break;

        case ptCOLFMT_RGBA8888:
            pBuf = (bbU32*) pSrc;
            break;

        default:
            bbErrSet(bbENOTSUP);
            goto ptSpriteConvert_err;
        }

        pSrc += pSrcSprite->stride;

        // write a line to the destination sprite in desired bit depth

        switch (depth)
        {
        case 4:
            {
                #if pt16BPP == pt16BPP_BGR555
                bbU32 const mask = ~0x7FF00UL;
                #endif

                register bbU16* pTmp = (bbU16*) pDst;
                bbU16* const pTmpEnd = pTmp + pSrcSprite->width;

                while (pTmp < pTmpEnd)
                {
                    register bbU32 rgb32 = *(pBuf++);

                    #if pt16BPP == pt16BPP_BGR555

                    register bbUINT rgb15 = (rgb32>>6) & 0x3E0U; // G
                    rgb32 &= mask;
                    rgb15 |= rgb32 >> 3; // R
                    rgb15 |= rgb32 >> 9; // B
                    //MOV R8,#31<<5
                    //AND R1,R8,R0,LSR #3+3-5
                    //BIC R0,R0,#&7FF00
                    //ORR R1,R1,R0,LSR #3
                    //ORR R1,R1,R0,LSR #9

                    #elif pt16BPP == pt16BPP_RGB565

                    register bbUINT rgb15 = (bbUINT)(rgb32>>5) & 0x7E0U; // G 6 bit
                    rgb15 |= (bbUINT)(rgb32>>19); // B 5 bit
                    rgb32 &= 31U<<3;
                    rgb15 |= (rgb32<<8); // R 5 bit
                    //register bbUINT rgb15 = 0;
                    //rgb15 |= ((rgb32>>3)&31)<<11;
                    //rgb15 |= ((rgb32>>(2+8))&63)<<5;
                    //rgb15 |= ((rgb32>>(3+16))&31);

                    #endif

                    *(pTmp++) = (bbU16) rgb15;
                }
            }
            break;

        default:
            bbErrSet(bbENOTSUP);
            goto ptSpriteConvert_err;
        }

        pDst += pDstSprite->stride;
    }

    return pDstSprite;

    ptSpriteConvert_err:
    bbMemFreeNull((void**)&pLineBuf);
    ptSpriteConvert_err2:
    bbMemFree(pDstSprite);
    return NULL;
}
