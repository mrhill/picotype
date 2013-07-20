#include "ptCol.h"
#include "ptPal.h"
#include <babel/mem.h>

ptColFmtInfo ptgColFmtInfo[ptCOLFMTCOUNT] = { ptCOLFMTINFO };

ptCOLTYPE ptColFmtGetType(ptCOLFMT colfmt)
{
    if (ptColFmtIsYUV(colfmt))
        return ptCOLTYPE_YUV;

    if (ptColFmtIsIndexed(colfmt))
        return ptCOLTYPE_PAL;

    return ptCOLTYPE_RGB;
}

ptRGBA ptYUV2RGB::ToRGB(ptYUVA yuva) const
{
    int const y = (int)ptYUVA_Y(yuva) + (int)mYUV2RGB[0];
    int const u = (int)ptYUVA_U(yuva) + (int)mYUV2RGB[1];
    int const v = (int)ptYUVA_V(yuva) + (int)mYUV2RGB[2];
    int rgb;

    register int p;

    if ((p = (y * mYUV2RGB[3] + u * mYUV2RGB[4] + v * mYUV2RGB[5]) >> 10) < 0) p=0;
    if (p>255) p=255;
    rgb = p; // R

    if ((p = (y * mYUV2RGB[6] + u * mYUV2RGB[7] + v * mYUV2RGB[8]) >> 10) < 0) p=0;
    if (p>255) p=255;
    rgb |= p<<8; // G

    if ((p = (y * mYUV2RGB[9] + u * mYUV2RGB[10]+ v * mYUV2RGB[11]) >> 10) < 0) p=0;
    if (p>255) p=255;
    rgb |= p<<16; // B

    return rgb | (yuva & 0xFF000000U);
}

void ptYUV2RGB::SwapUV()
{
    for(int i=0; i<=9; i+=3) // swap UV coeffs
    {
        bbS16 tmp = mYUV2RGB[i+2];
        mYUV2RGB[i+2] = mYUV2RGB[i+1];
        mYUV2RGB[i+1] = tmp;
    }
}

void ptYUV2RGB::MaskChannel(bbUINT ch)
{
    mYUV2RGB[ch+3] = mYUV2RGB[ch+6] = mYUV2RGB[ch+9] = 0;
}

ptRGB2YUV::ptRGB2YUV(const ptYUV2RGB& yuv2rgb)
{
    mRGB2YUV[0] = -yuv2rgb.mYUV2RGB[0];
    mRGB2YUV[1] = -yuv2rgb.mYUV2RGB[1];
    mRGB2YUV[2] = -yuv2rgb.mYUV2RGB[2];

    const bbS16* m = yuv2rgb.GetMatrix();
    #define A(x,y) ((double)(int)m[3+x+y*3]/(double)(1<<10))
    #define toFix(d) (bbS16)(d*(1<<10))
    #define OUT(y,x) mRGB2YUV[3+x+y*3]

    double det = A(0,0)*(A(1,1)*A(2,2)-A(2,1)*A(1,2)) - A(0,1)*(A(1,0)*A(2,2)-A(1,2)*A(2,0)) + A(0,2)*(A(1,0)*A(2,1)-A(1,1)*A(2,0));
    if (det == 0)
        return;
    det = 1/det;
    OUT(0,0) = toFix( (A(1,1)*A(2,2)-A(2,1)*A(1,2))*det);
    OUT(1,0) = toFix(-(A(0,1)*A(2,2)-A(0,2)*A(2,1))*det);
    OUT(2,0) = toFix( (A(0,1)*A(1,2)-A(0,2)*A(1,1))*det);
    OUT(0,1) = toFix(-(A(1,0)*A(2,2)-A(1,2)*A(2,0))*det);
    OUT(1,1) = toFix( (A(0,0)*A(2,2)-A(0,2)*A(2,0))*det);
    OUT(2,1) = toFix(-(A(0,0)*A(1,2)-A(1,0)*A(0,2))*det);
    OUT(0,2) = toFix( (A(1,0)*A(2,1)-A(2,0)*A(1,1))*det);
    OUT(1,2) = toFix(-(A(0,0)*A(2,1)-A(2,0)*A(0,1))*det);
    OUT(2,2) = toFix( (A(0,0)*A(1,1)-A(1,0)*A(0,1))*det);

    #undef OUT
    #undef toFix
    #undef A
}

ptYUVA ptRGB2YUV::ToYUVA(ptRGBA rgba) const
{
    int const r = ptRGBA_R(rgba);
    int const g = ptRGBA_G(rgba);
    int const b = ptRGBA_B(rgba);
    int yuv;

    register int p;

    p = (r * mRGB2YUV[3] + g * mRGB2YUV[4] + b * mRGB2YUV[5]) >> 10;
    if ((p+=(int)mRGB2YUV[0]) < 0) p=0;
    if (p>255) p=255;
    yuv = p; // Y

    p = (r * mRGB2YUV[6] + g * mRGB2YUV[7] + b * mRGB2YUV[8]) >> 10;
    if ((p+=(int)mRGB2YUV[1]) < 0) p=0;
    if (p>255) p=255;
    yuv |= p<<8; // U

    p = (r * mRGB2YUV[9] + g * mRGB2YUV[10]+ b * mRGB2YUV[11]) >> 10;
    if ((p+=(int)mRGB2YUV[2]) < 0) p=0;
    if (p>255) p=255;
    yuv |= p<<16; // V

    return yuv | (rgba & 0xFF000000U);
}

