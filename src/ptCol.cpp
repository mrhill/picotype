#include "ptCol.h"
#include "ptPal.h"

ptColFmtInfo ptgColFmtInfo[ptCOLFMTCOUNT] = { ptCOLFMTINFO };

ptCOLTYPE ptColFmtGetType(ptCOLFMT colfmt)
{
    if (ptColFmtIsYUV(colfmt))
        return ptCOLTYPE_YUV;

    if (ptColFmtIsIndexed(colfmt))
        return ptCOLTYPE_PAL;
    
    return ptCOLTYPE_RGB;
}

ptRGBA ptYUV2RGB::ToRGB(ptYUVA yuv) const
{
    int const y = (int)ptYUVA_Y(yuv) + (int)mYUV2RGB[0];
    int const u = (int)ptYUVA_U(yuv) + (int)mYUV2RGB[1];
    int const v = (int)ptYUVA_V(yuv) + (int)mYUV2RGB[2];
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

    return rgb;
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

