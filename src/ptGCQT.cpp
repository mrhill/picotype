#include "ptGCQT.h"

#if ptUSE_GCQT == 1

#include <QPaintDevice>
#include <QPainter>
#include <QBrush>
#include <QImage>

ptGCQT::ptGCQT(QPainter* pPainter)
{
    mpLineCache = NULL;
    AttachPainter(pPainter);
}

ptGCQT::~ptGCQT()
{
    delete mpLineCache;
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
    if (mpLineCache && (width <= (bbUINT)mpLineCache->width()) && (height <= (bbUINT)mpLineCache->height()))
        return bbEOK;
    delete mpLineCache;

    mpLineCache = new QImage(width, height, QImage::Format_Indexed8);
    if (!mpLineCache)
        return bbErrSet(bbENOMEM);

    mpLineCache->setColorCount(256);
    const bbU32* const pRGB = mpLogPal->mpRGB;
    for(int i=0; i<256; i++)
    {
        bbU32 rgb = pRGB[i];
        mpLineCache->setColor(i, qRgb(rgb&0xFF, (rgb>>8)&0xFF, (rgb>>16)&0xFF));
    }

    return bbEOK;
}

void ptGCQT::SetClipBox(const int /*clipminx*/, const int /*clipminy*/, const int /*clipmaxx*/, const int /*clipmaxy*/) {}
void ptGCQT::GetClipBox(ptRect* const /*pRect*/) {}
void ptGCQT::Clear(const bbUINT /*col*/) {}
void ptGCQT::Point(const int /*x*/, const int /*y*/, const bbUINT /*col*/) {}
void ptGCQT::HLine(int /*x*/, int /*y*/, bbUINT /*width*/, bbUINT /*col*/) {}
void ptGCQT::VLine(int /*x*/, int /*y*/, bbUINT /*height*/, bbUINT /*col*/) {}

void ptGCQT::Box(const int x, const int y, const bbUINT width, const bbUINT height, const bbUINT col)
{
    const bbU32 rgba = mpLogPal->mpRGB[col & ptPENCOLMASK];
    const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
    qDrawPlainRect(mpPainter, x>>ptGCEIGHTX, y>>ptGCEIGHTY, width>>ptGCEIGHTX, height>>ptGCEIGHTY, rgb);
}

void ptGCQT::FillBox(int x, int y, bbUINT width, bbUINT height, ptPEN pen)
{
    const bbU32 rgba = mpLogPal->mpRGB[pen & ptPENCOLMASK];
    const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
    mpPainter->fillRect(x>>ptGCEIGHTX, y>>ptGCEIGHTY, width>>ptGCEIGHTX, height>>ptGCEIGHTY, rgb);
}

void ptGCQT::Line(int x1, int y1, int x2, int y2, const ptPEN pen)
{
    const bbU32 rgba = mpLogPal->mpRGB[pen & ptPENCOLMASK];
    const QColor rgb = QColor(rgba & 0xFF, (rgba>>8) & 0xFF, (rgba>>16) & 0xFF);
    mpPainter->setPen(rgb);
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

    //xxx CachePal(mpLogPal, 0);

    for(;;)
    {
        bbU32 cp = *(pText++);
        bbU32 cp2 = *pText & 0x3FFFFFUL;

        if (cp2 == 0x3FFFFEUL)
            ++pText;

        if (cp == 0xFFFFFFFFUL) // EOL?
        {
            cp = (bbU32)pData - (bbU32)pDataStart;
            if (cp > lineCrop)
                cp = lineCrop;
            mpPainter->drawImage(QPoint(x, y), *mpLineCache, QRect(0, 0, cp, height));

            return (x - x_org + (DWORD)pData - (DWORD)pDataStart) << ptGCEIGHTX;
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

        if (((bbU32)pData-(bbU32)pDataStart+tmp) <= lineCrop)
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
bbUINT ptGCQT::Text(int /*x*/, int /*y*/, const bbCHAR* /*pMarkup*/, bbUINT /*fgcol*/, ptPEN /*bgpen*/, bbUINT const /*font*/) { return 0; }
void ptGCQT::Sprite(int /*x*/, int /*y*/, const ptSprite* const /*pSprite*/) {}

#endif
