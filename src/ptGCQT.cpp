#include "ptGCQT.h"

#if ptUSE_GCQT == 1

#include <QPaintDevice>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include "ptSprite.h"
#include "ptconvert.h"

ptGCQT::ptGCQT(QPainter* pPainter)
{
    mpSpriteBuf =
    mpLineCache = NULL;
    AttachPainter(pPainter);
    mLogPalHash = 0;
    mPalHash = 0;
    mPal.resize(256);
}

ptGCQT::~ptGCQT()
{
    delete mpLineCache;
    delete mpSpriteBuf;
}

void ptGCQT::AttachPainter(QPainter* pPainter)
{
    mpPainter = pPainter;
    if (pPainter)
    {
        QPaintDevice* pDev = pPainter->device();
        if (pDev)
        {
            mWidth = pDev->width() << ptGCEIGHTX;
            mHeight = pDev->height() << ptGCEIGHTY;
        }
        pPainter->setBackground(Qt::black);
        pPainter->setBackgroundMode(Qt::OpaqueMode);
    }
}

bbUINT ptGCQT::GetWidth() const
{
    return mWidth;
}

bbUINT ptGCQT::GetHeight() const
{
    return mHeight;
}

bbERR ptGCQT::EnsureLineCache(bbUINT width, bbUINT height)
{
    bbU32 const newhash = (bbU32)(bbUPTR)mpLogPal ^ mpLogPal->mSyncPt;

    if (mpLineCache &&
        (width <= (bbUINT)mpLineCache->width()) &&
        (height <= (bbUINT)mpLineCache->height()) &&
        (mLogPalHash == newhash))
    {
        return bbEOK;
    }

    delete mpLineCache;

    mpLineCache = new QImage(width, height, QImage::Format_Indexed8);
    if (!mpLineCache)
        return bbErrSet(bbENOMEM);

    mLogPalHash = newhash;
    mpLineCache->setColorCount(256);
    const bbU32* const pRGB = mpLogPal->mpRGB;
    for(int i=0; i<256; i++)
    {
        bbU32 rgb = pRGB[i];
        mpLineCache->setColor(i, qRgb(rgb&0xFF, (rgb>>8)&0xFF, (rgb>>16)&0xFF));
    }

    return bbEOK;
}

void ptGCQT::SetClipBox(const int x, const int y, const int maxx, const int maxy)
{
    mpPainter->setClipRect(x, y, maxx-x, maxy-y);
    if ((x<=0) && (y<=0) && (maxx>=(int)mWidth) && (maxy>=(int)mHeight))
        mpPainter->setClipping(false);
}

void ptGCQT::GetClipBox(ptRect* const pRect)
{
    QRect r = mpPainter->clipRegion().boundingRect();
    if (r.isNull())
    {
        pRect->left = pRect->top = 0;
        pRect->right = GetWidth();
        pRect->bottom= GetHeight();
    }
    else
    {
        pRect->left = r.x();
        pRect->top = r.y();
        pRect->right = r.x() + r.width();
        pRect->bottom= r.y() + r.height();
    }
}

void ptGCQT::Clear(const bbUINT col)
{
    FillBox(0, 0, GetWidth(), GetHeight(), col);
}

void ptGCQT::Point(const int x, const int y, const bbUINT col)
{
    const bbU32 rgba = mpLogPal->mpRGB[col & ptPENCOLMASK];
    const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
    mpPainter->setPen(QPen(rgb));
    mpPainter->drawPoint(x, y);
}

void ptGCQT::HLine(int x, int y, bbUINT width, bbUINT col)
{
    Line(x, y, x+width, y, col);
}

void ptGCQT::VLine(int x, int y, bbUINT height, bbUINT col)
{
    Line(x, y, x, y+height, col);
}

void ptGCQT::Box(const int x, const int y, const bbUINT width, const bbUINT height, const bbUINT col)
{
    const bbU32 rgba = mpLogPal->mpRGB[col & ptPENCOLMASK];
    const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
    qDrawPlainRect(mpPainter, x>>ptGCEIGHTX, y>>ptGCEIGHTY, width>>ptGCEIGHTX, height>>ptGCEIGHTY, rgb);
}

void ptGCQT::FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen)
{
    if ((pen & ptPEN_ROP) && ((pen>>ptPENBITPOS_OPT2) == ptROP_NOT))
    {
        QPainter::CompositionMode cMode = mpPainter->compositionMode();
        mpPainter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        mpPainter->fillRect(x>>ptGCEIGHTX, y>>ptGCEIGHTY, width>>ptGCEIGHTX, height>>ptGCEIGHTY, QColor(0xFF,0xFF,0xFF));
        mpPainter->setCompositionMode(cMode);
    }
    else
    {
        const bbU32 rgba = mpLogPal->mpRGB[pen & ptPENCOLMASK];
        const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
        mpPainter->fillRect(x>>ptGCEIGHTX, y>>ptGCEIGHTY, width>>ptGCEIGHTX, height>>ptGCEIGHTY, rgb);
    }
}

void ptGCQT::Line(int x1, int y1, int x2, int y2, const ptPEN pen)
{
    if (pen & ptPEN_ROP)
    {
        QPainter::CompositionMode cMode = mpPainter->compositionMode();
        mpPainter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        mpPainter->setPen(QColor(0xFF,0xFF,0xFF));
        mpPainter->drawLine(x1, y1, x2, y2);
        mpPainter->setCompositionMode(cMode);
        return;
    }

    if (pen & ptPEN_PAT)
    {
        const bbU32 rgba = mpLogPal->mpRGB[pen & ptPENCOLMASK];
        QPen pen = QPen(QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF));
        pen.setStyle(Qt::DashLine);
        mpPainter->setPen(pen);
    }
    else
    {
        const bbU32 rgba = mpLogPal->mpRGB[pen & ptPENCOLMASK];
        const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
        mpPainter->setPen(rgb);
    }

    mpPainter->drawLine(x1, y1, x2, y2);
}

bbUINT ptGCQT::MarkupText(int x, int y, const bbU32* pText, const ptMarkupInfo* const pInfo, bbUINT const linespacing)
{
    x>>=ptGCEIGHTX;
    y>>=ptGCEIGHTY;
    int const x_org = x;

    ptFont* pFont = pInfo->mpFont[0];
    const bbUINT height = pFont->GetHeight() + linespacing;
    const bbUINT lineCrop = (mWidth >> ptGCEIGHTX) + ptFONT_MAXWIDTH*2;
    if (EnsureLineCache(lineCrop, height) != bbEOK)
        return 0;
    const bbUINT pitch = mpLineCache->bytesPerLine();
    bbU8* const pDataStart = mpLineCache->bits();
    bbU8* pData = pDataStart;
    ptPEN bgpen = 0;

    //xxx CachePal(mpLogPal, 0);

    for(;;)
    {
        bbU32 cp = *(pText++);
        bbU32 cp2 = *pText & 0x3FFFFFUL;

        if (cp2 == 0x3FFFFEUL)
            ++pText;

        if (cp == 0xFFFFFFFFUL) // EOL?
        {
            cp = (bbU32)(bbUPTR)pData - (bbU32)(bbUPTR)pDataStart;
            if (cp > lineCrop)
                cp = lineCrop;

            QRgb oldColor;

            if (bgpen & ptPEN_TRANS)
            {
                oldColor = mpLineCache->color(bgpen & 0xFF);
                mpLineCache->setColor(bgpen & 0xFF, qRgba(0, 0, 0, 0));
            }
            mpPainter->drawImage(QPoint(x, y), *mpLineCache, QRect(0, 0, cp, height));
            if (bgpen & ptPEN_TRANS)
                mpLineCache->setColor(bgpen & 0xFF, oldColor);

            return (x - x_org + (bbU32)(bbUPTR)pData - (bbU32)(bbUPTR)pDataStart) << ptGCEIGHTX;
        }

        bbUINT const fgcol = pInfo->mFGCol[(cp >> ptGCMT_FGPOS) & ptGCMT_FGMASK];
        bgpen = pInfo->mBGPen[cp >> ptGCMT_BGPOS];
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

        if (((bbU32)(bbUPTR)pData-(bbU32)(bbUPTR)pDataStart+tmp) <= lineCrop)
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

void ptGCQT::Polygon(const ptCoord* const /*pPoly*/, const bbUINT /*points*/, const ptPEN /*pen*/) {}
void ptGCQT::FillCircle(int /*x*/, int /*y*/, bbUINT /*rad*/, const ptPEN /*pen*/) {}

void ptGCQT::CachePal(ptPal* const pPal, bbUINT size)
{
    bbU32 const newhash = (bbU32)(bbUPTR)pPal ^ pPal->mSyncPt;

    if (newhash == mPalHash)
        return;

    mPalHash = newhash;

    if (!size)
        size = pPal->mColCount;

    bbASSERT(size <= pPal->mColCount);
    bbASSERT((size-1) < 256);

    const bbU32* const pRGB = pPal->mpRGB;
    do
    {
        bbU32 rgb = pRGB[--size];
        mPal[size] = qRgb(rgb&0xFF, (rgb>>8)&0xFF, (rgb>>16)&0xFF);
    } while(size);
}

bbERR ptGCQT::EnsureSpriteBuf(bbUINT width, bbUINT height, QImage::Format fmt)
{
    if (!mpSpriteBuf ||
        (mpSpriteBuf->format() != fmt) ||
        ((bbUINT)mpSpriteBuf->width() < width) ||
        ((bbUINT)mpSpriteBuf->height() < height))
    {
        delete mpSpriteBuf;
        mpSpriteBuf = new QImage(width, height, fmt);
        if (!mpSpriteBuf)
            return bbErrSet(bbENOMEM);
    }
    return bbEOK;
}

void ptGCQT::Sprite(int x, int y, const ptSprite* const pSprite)
{
    bbU32 i;
    bbU8* pData = pSprite->pData;
    uchar* pBits;
    const bbS16* pYUV2RGB;

    x>>=ptGCEIGHTX;
    y>>=ptGCEIGHTY;
    int const y_end = y + pSprite->GetHeight();

    bbS16 yuv2rgb[12];

    if (ptColFmtIsYUV(pSprite->GetColFmt()))
    {
        pYUV2RGB = pSprite->pYUV2RGB;
        if (ptgColFmtInfo[pSprite->GetColFmt()].flags & ptCOLFMTFLAG_SWAPUV) // VU order?
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

    switch (pSprite->GetColFmt())
    {
    case ptCOLFMT_1BPP:
        {
        QImage::Format const fmt = (pSprite->GetBitOrder() == ptBITORDER_LSBLEFT) ? QImage::Format_MonoLSB : QImage::Format_Mono;
        QImage image((const uchar*)pSprite->pData, pSprite->GetWidth(), pSprite->GetHeight(), pSprite->GetStride(), fmt);
        CachePal(pSprite->pPal, 0);
        image.setColorTable(mPal);
        mpPainter->drawImage(QPoint(x, y), image);
        }
        break;
    case ptCOLFMT_2BPP:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_Indexed8))
            return;
        CachePal(pSprite->pPal, 0);
        mpSpriteBuf->setColorTable(mPal);
        while (y < y_end)
        {
            ptExpand_2BppTo8Bpp(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetBitOrder()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_4BPP:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_Indexed8))
            return;
        CachePal(pSprite->pPal, 0);
        mpSpriteBuf->setColorTable(mPal);
        while (y < y_end)
        {
            ptExpand_4BppTo8Bpp(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetBitOrder()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_8BPP:
        {
        QImage image((const uchar*)pSprite->pData, pSprite->GetWidth(), pSprite->GetHeight(), pSprite->GetStride(), QImage::Format_Indexed8);
        CachePal(pSprite->pPal, 0);
        image.setColorTable(mPal);
        mpPainter->drawImage(QPoint(x, y), image);
        }
        break;
    case ptCOLFMT_2BPPP:
    case ptCOLFMT_3BPPP:
    case ptCOLFMT_4BPPP:
    case ptCOLFMT_5BPPP:
    case ptCOLFMT_6BPPP:
    case ptCOLFMT_7BPPP:
    case ptCOLFMT_8BPPP:
        {
        bbU32 offset = 0;
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_Indexed8))
            return;
        CachePal(pSprite->pPal, 0);
        mpSpriteBuf->setColorTable(mPal);
        while (y < y_end)
        {
            pBits = mpSpriteBuf->bits();
            ptExpand_1BppTo8Bpp(pData+offset, pBits, pSprite->GetWidth(), pSprite->GetBitOrder());
            bbUINT planeCount = ptgColFmtInfo[pSprite->GetColFmt()].PlaneCount;
            i = 1;
            do
            {
                ptMerge_1BppTo8Bpp(pSprite->GetPlane(i)+offset, pBits, pSprite->GetWidth(), pSprite->GetBitOrder(), i);
            } while (++i < planeCount);
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
            offset += pSprite->GetStride();
        }
        }
        break;
    case ptCOLFMT_RGB565:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB32))
            return;
        while (y < y_end)
        {
            ptConvert_RGB565ToBGRA8888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetEndian()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_RGBA1555:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB32))
            return;
        while (y < y_end)
        {
            ptConvert_RGBA1555ToBGRA8888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetEndian()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_RGBA4444:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB32))
            return;
        while (y < y_end)
        {
            ptConvert_RGBA4444ToBGRA8888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetEndian()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_BGR888:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB888))
            return;
        while (y < y_end)
        {
            ptConvert_BGR888ToRGB888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetEndian()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_RGB888:
        {
        QImage image((const uchar*)pSprite->pData, pSprite->GetWidth(), pSprite->GetHeight(), pSprite->GetStride(), QImage::Format_RGB888);
        mpPainter->drawImage(QPoint(x, y), image);
        }
        break;
    case ptCOLFMT_RGBA8888:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB32))
            return;
        while (y < y_end)
        {
            ptConvert_BGRA8888ToRGBA8888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pSprite->GetEndian()); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_BGRA8888:
        {
        QImage image((const uchar*)pSprite->pData, pSprite->GetWidth(), pSprite->GetHeight(), pSprite->GetStride(), QImage::Format_RGB32);
        mpPainter->drawImage(QPoint(x, y), image);
        }
        break;
    case ptCOLFMT_YUV420P:
    case ptCOLFMT_YUV420P_YV12:
    case ptCOLFMT_YUV420P_IMC1:
    case ptCOLFMT_YUV420P_IMC3:
    case ptCOLFMT_YUV420P_IMC4:
    case ptCOLFMT_YUV420P_IMC2:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 2, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUV420ToRGB888(pSprite->GetPlane(0) + offsetY,
                                         (y_end-y) < 2 ? NULL : pSprite->GetPlane(1) + offsetY,
                                         pSprite->GetPlane(2) + offsetUV,
                                         pSprite->GetPlane(3) + offsetUV,
                                         mpSpriteBuf->bits(),
                                         mpSpriteBuf->scanLine(1),
                                         width,
                                         pYUV2RGB);

                mpPainter->drawImage(QPoint(x, y), *mpSpriteBuf);
                offsetY  += pSprite->GetStride()<<1;
                offsetUV += pSprite->GetStrideUV();
                y+=2;
            }
        }
        break;
    case ptCOLFMT_YUV420P_NV12:
    case ptCOLFMT_YUV420P_NV21:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 2, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUVNV12ToRGB888(pSprite->GetPlane(0) + offsetY,
                                          (y_end-y) < 2 ? NULL : pSprite->GetPlane(1) + offsetY,
                                          pSprite->GetPlane(2) + offsetUV,
                                          mpSpriteBuf->bits(),
                                          mpSpriteBuf->scanLine(1),
                                          width,
                                          pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y), *mpSpriteBuf);
                offsetY  += pSprite->GetStride()<<1;
                offsetUV += pSprite->GetStrideUV();
                y+=2;
            }
        }
        break;
    case ptCOLFMT_YUV420P_12:
    case ptCOLFMT_YUV420P_16:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 2, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUV42016ToRGB888(pSprite->GetPlane(0) + offsetY,
                                           (y_end-y) < 2 ? NULL : pSprite->GetPlane(1) + offsetY,
                                           pSprite->GetPlane(2) + offsetUV,
                                           pSprite->GetPlane(3) + offsetUV,
                                           mpSpriteBuf->bits(),
                                           mpSpriteBuf->scanLine(1),
                                           width,
                                           pYUV2RGB,
                                           pSprite->GetColFmt() == ptCOLFMT_YUV420P_12 ? 4 : 8,
                                           pSprite->GetEndian());

                mpPainter->drawImage(QPoint(x, y), *mpSpriteBuf);
                offsetY  += pSprite->GetStride()<<1;
                offsetUV += pSprite->GetStrideUV();
                y+=2;
            }
        }
        break;
	case ptCOLFMT_YUV411:
        {
            bbU32 width = (pSprite->GetWidth() + 3) &~ 3;

            if (bbEOK != EnsureSpriteBuf(width, 1, QImage::Format_RGB888))
                return;

            while (y < y_end)
            {
                ptConvert_YUV411ToRGB888(pData, mpSpriteBuf->bits(), width, pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
            }
        }
        break;
    case ptCOLFMT_YUYV:
    case ptCOLFMT_YVYU:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 1, QImage::Format_RGB888))
                return;

            while (y < y_end)
            {
                ptConvert_YUYVToRGB888(pData, mpSpriteBuf->bits(), width, pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
            }
        }
        break;
    case ptCOLFMT_UYVY:
    case ptCOLFMT_VYUY:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 1, QImage::Format_RGB888))
                return;

            while (y < y_end)
            {
                ptConvert_UYVYToRGB888(pData, mpSpriteBuf->bits(), width, pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
            }
        }
        break;
    case ptCOLFMT_YUV422P:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 1, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUV422PToRGB888(pSprite->GetPlane(0) + offsetY,
                                          pSprite->GetPlane(1) + offsetUV,
                                          pSprite->GetPlane(2) + offsetUV,
                                          mpSpriteBuf->bits(),
                                          width,
                                          pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
                offsetY += pSprite->GetStride();
                offsetUV += pSprite->GetStrideUV();
            }
        }
        break;
    case ptCOLFMT_YUV422RP:
        {
            bbU32 width = pSprite->GetWidth();
            width += width&1;

            if (bbEOK != EnsureSpriteBuf(width, 2, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUV422RPToRGB888(pSprite->GetPlane(0) + offsetY,
                                           (y_end-y) < 2 ? NULL : pSprite->GetPlane(1) + offsetY,
                                           pSprite->GetPlane(2) + offsetUV,
                                           pSprite->GetPlane(3) + offsetUV,
                                           mpSpriteBuf->bits(),
                                           mpSpriteBuf->scanLine(1),
                                           width,
                                           pYUV2RGB);
                mpPainter->drawImage(QPoint(x, y), *mpSpriteBuf);
                offsetY  += pSprite->GetStride()<<1;
                offsetUV += pSprite->GetStrideUV();
                y+=2;
            }
        }
        break;
    case ptCOLFMT_YUV444:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB888))
            return;
        while (y < y_end)
        {
            ptConvert_YUV444ToRGB888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pYUV2RGB); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    case ptCOLFMT_YUV444P:
        {
            if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB888))
                return;

            bbU32 offsetY = 0;
            bbU32 offsetUV = 0;

            while (y < y_end)
            {
                ptConvert_YUV444PToRGB888(pSprite->GetPlane(0) + offsetY,
                                          pSprite->GetPlane(1) + offsetUV,
                                          pSprite->GetPlane(2) + offsetUV,
                                          mpSpriteBuf->bits(),
                                          pSprite->GetWidth(),
                                          pYUV2RGB);

                mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
                offsetY += pSprite->GetStride();
                offsetUV += pSprite->GetStrideUV();
            }
        }
        break;
    case ptCOLFMT_AYUV:
        if (bbEOK != EnsureSpriteBuf(pSprite->GetWidth(), 1, QImage::Format_RGB888))
            return;
        while (y < y_end)
        {
            ptConvert_AYUVToRGB888(pData, mpSpriteBuf->bits(), pSprite->GetWidth(), pYUV2RGB); pData+=pSprite->GetStride();
            mpPainter->drawImage(QPoint(x, y++), *mpSpriteBuf);
        }
        break;
    default:
        return;
    }
}

#endif

