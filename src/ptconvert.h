#ifndef ptCONVERT_H_
#define ptCONVERT_H_

#include "ptdefs.h"

void ptMakePal2PalLookup(const ptPal* pSrc, const ptPal* pDst, bbU8* pLU);
void ptExpand_4BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder);
void ptExpand_2BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder);
void ptExpand_1BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder);
void ptMerge_1BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptBITORDER srcOrder, bbUINT shift);
void ptConvert_8BppTo2BppLSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU);
void ptConvert_8BppTo4BppLSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU);
void ptConvert_8BppTo4BppMSB(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU);
void ptConvert_8BppTo8Bpp(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbU8* pColIdxLU);
void ptConvert_RGB565ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGB565ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGB565ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGBA1555ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGBA1555ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGBA1555ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGBA4444ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGBA4444ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGBA4444ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGB888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_BGR888ToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_BGR888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_BGRA8888ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, ptENDIAN endian);
void ptConvert_RGBA8888ToRGB565(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGBA8888ToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGBA8888ToBGR888(const bbU8* pSrc, bbU8* pDst, bbU32 width);
void ptConvert_RGBA8888ToBGRA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width);

void ptConvert_YUV420ToRGB888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU8* pDst2, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV420ToRGBA8888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV42016ToRGB888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU8* pDst2, bbU32 width, const bbS16* pYUV2RGB, unsigned shift, ptENDIAN endian);
void ptConvert_YUV42016LEToRGBA8888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB, unsigned shift);
void ptConvert_YUV42016BEToRGBA8888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB, unsigned shift);
void ptConvert_YUVNV12ToRGBA8888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcUV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUVNV12ToRGB888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcUV, bbU8* pDst, bbU8* pDst2, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUYVToRGB888(const bbU8* pSrcY, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUYVToRGBA8888(const bbU8* pSrcY, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_UYVYToRGB888(const bbU8* pSrcY, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB); 
void ptConvert_UYVYToRGBA8888(const bbU8* pSrcY, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV422PToRGB888(const bbU8* pSrcY, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV422PToRGBA8888(const bbU8* pSrcY, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV422RPToRGB888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU8* pDst2, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV422RPToRGBA8888(const bbU8* pSrcY0, const bbU8* pSrcY1, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV444ToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV444PToRGB888(const bbU8* pSrcY, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV444PToRGBA8888(const bbU8* pSrcY, const bbU8* pSrcU, const bbU8* pSrcV, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_YUV444ToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_AYUVToRGBA8888(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);
void ptConvert_AYUVToRGB888(const bbU8* pSrc, bbU8* pDst, bbU32 width, const bbS16* pYUV2RGB);

#endif /* ptCONVERT_H_ */

