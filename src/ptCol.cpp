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

ptRGB2YUV::ptRGB2YUV(const ptYUV2RGB& yuv2rgb)
{
    bbMemCpy(mRGB2YUV, yuv2rgb.mYUV2RGB, sizeof(mRGB2YUV)); ///xxx
}

ptYUVA ptRGB2YUV::ToYUVA(ptRGBA rgba) const
{
    int const r = ptRGBA_R(rgba);
    int const g = ptRGBA_G(rgba);
    int const b = ptRGBA_B(rgba);
    int yuv = rgba; //xxx

    return yuv | (rgba & 0xFF000000U);
}

