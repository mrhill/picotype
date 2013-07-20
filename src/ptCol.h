#ifndef ptCOL_H
#define ptCOL_H

#include "ptdefs.h"
#include <babel/mem.h>

typedef bbU32 ptRGBA;

inline bbU32  ptRGBA_R(ptRGBA c) { return c&0xFF; }
inline bbU32  ptRGBA_G(ptRGBA c) { return (c>>8)&0xFF; }
inline bbU32  ptRGBA_B(ptRGBA c) { return (c>>16)&0xFF; }
inline bbU32  ptRGBA_A(ptRGBA c) { return c>>24; }
inline ptRGBA ptRGBA_SetR(ptRGBA c, bbU32 r) { return (c & 0xFFFFFF00U) | r; }
inline ptRGBA ptRGBA_SetG(ptRGBA c, bbU32 g) { return (c & 0xFFFF00FFU) | (g<<8); }
inline ptRGBA ptRGBA_SetB(ptRGBA c, bbU32 b) { return (c & 0xFF00FFFFU) | (b<<16); }
inline ptRGBA ptRGBA_SetA(ptRGBA c, bbU32 a) { return (c & 0x00FFFFFFU) | (a<<24); }
inline ptRGBA ptRGBA_Set(bbU32 r, bbU32 g, bbU32 b, bbU32 a) { return r|(g<<8)|(b<<16)|(a<<24); }

typedef bbU32 ptYUVA;

inline bbU32 ptYUVA_Y(ptYUVA c) { return c&0xFF; }
inline bbU32 ptYUVA_U(ptYUVA c) { return (c>>8)&0xFF; }
inline bbU32 ptYUVA_V(ptYUVA c) { return (c>>16)&0xFF; }
inline bbU32 ptYUVA_A(ptYUVA c) { return c>>24; }
inline ptYUVA ptYUVA_SetY(ptYUVA c, bbU32 y) { return (c & 0xFFFFFF00U) | y; }
inline ptYUVA ptYUVA_SetU(ptYUVA c, bbU32 u) { return (c & 0xFFFF00FFU) | (u<<8); }
inline ptYUVA ptYUVA_SetV(ptYUVA c, bbU32 v) { return (c & 0xFF00FFFFU) | (v<<16); }
inline ptYUVA ptYUVA_SetA(ptYUVA c, bbU32 a) { return (c & 0x00FFFFFFU) | (a<<24); }
inline ptYUVA ptYUVA_Set(bbU32 y, bbU32 u, bbU32 v, bbU32 a) { return y|(u<<8)|(v<<16)|(a<<24); }

class ptYUV2RGB
{
    bbS16 mYUV2RGB[12];// must be first member for now, see ptSprite::pYUV2RGB
public:
    ptYUV2RGB(const bbS16* pM) { bbMemCpy(mYUV2RGB, pM, sizeof(mYUV2RGB)); }
    ptYUV2RGB() {}

    const bbS16* GetMatrix() const { return mYUV2RGB; }

    ptRGBA ToRGB(ptYUVA yuva) const;

    void SwapUV();
    void MaskChannel(bbUINT ch);

    friend class ptRGB2YUV;
};

class ptRGB2YUV
{
    bbS16 mRGB2YUV[12];
public:
    ptRGB2YUV() {}
    ptRGB2YUV(const ptYUV2RGB& yuv2rgb);

    const bbS16* GetMatrix() const { return mRGB2YUV; }

    ptYUVA ToYUVA(ptRGBA rgba) const;

    friend class ptYUV2RGB;
};

#endif

