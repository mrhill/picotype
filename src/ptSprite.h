#ifndef ptSPRITE_H_
#define ptSPRITE_H_

#include <babel/defs.h>
#include <babel/mem.h>
#include "ptdefs.h"

/** Flagbits for ptSprite::flags. */
enum ptSPRITEFLAG
{
    ptSPRITEFLAG_TRANSPARENT = 0x01, //!< Colour 0 of sprite is transparent.
};

/** Sprite for use in graphics context #ptGC.
    Sprites of bit depth 1, 2, 4, and 8 bpp are palette indexed.
    Sprites with planar colour format have plane stride of stride*height
*/
struct ptSprite
{
    bbU32   width;          //!< Width in pixels
    bbU32   height;         //!< Height in pixels
    bbU32   stride;         //!< Stride of a rasterline in bytes, for planar formats this is the stride in the 1st plane
    bbU32   strideUV;       //!< Stride of a U,V rasterline in bytes, for planar YUV formats only
    bbU8    colfmt;         //!< ptCOLFMT
    bbU8    endian;         //!< ptENDIAN
    bbU8    bitorder;       //!< ptBITORDER
    bbU8    flags;          //!< Flag bits, see ptSPRITEFLAG

    union
    {
        ptPal* pPal;          /**< Pointer to palette, used for paletized colour formats only.
                                   The object is under external control. */
        ptYUV2RGB* poYUV2RGB; /**< Pointer to YUV2RGB conversion matrix, used for YUV colour formats only. */
        const bbS16* pYUV2RGB;/**< Pointer to YUV2RGB conversion 3x4 matrix, used for YUV colour formats only.
                                   Row 0 is YUV 0-level normalization offset,
                                   Row 1-3 is YUV to RGB matrix. S.5.10 fixpoint values. */
    };

    union
    {
        bbU8*   pData;      /**< Chunky formats: pointer to pixel data, . */
        bbU8*   pPlane[8];  /**< Planar formats: pointers to pixel data for each plane.
                                 <table>
                                 <tr><th>Color format</th><th>Interpretation</th></tr>

                                 <tr><td>ptCOLFMT_YUV420P (I420)<br>
                                         ptCOLFMT_YUV420P_YV12 <br>
                                         ptCOLFMT_YUV420P_IMC3 <br>
                                         ptCOLFMT_YUV420P_IMC1 <br>
                                         ptCOLFMT_YUV420P_IMC4 <br>
                                         ptCOLFMT_YUV420P_IMC2 </td>
                                     <td>pPlane[0] even Y lines, byte offset to next even line is ptSprite::stride*2 <br>
                                         pPlane[1] odd  Y lines, byte offset to next odd  line is ptSprite::stride*2 <br>
                                         pPlane[2] U lines for I420, IMC3, IMC4, 
                                                   V lines for YV12, IMC1, IMC2, byte stride is in ptSprite::strideUV <br>
                                         pPlane[3] V lines for I420, IMC3, IMC4, 
                                                   U lines for YV12, IMC1, IMC2, byte stride is in ptSprite::strideUV <br>
                                         <br>
                                         For IMC3 and IMC1 strideUV specifies the stride including the unused padding area.
                                         For IMC4 and IMC2 strideUV specifies the stride including space for the interleaving
                                         V/U data, and pPlane[3] points into the right half of the interleaved U/V data.
                                         <br>
                                         By adhering to this conventions, there is no difference between the YUV420 planar
                                         formats from the data access implementation point of view.
                                     </td></tr>
                                 <tr><td>ptCOLFMT_YUV420P_NV12 <br>
                                         ptCOLFMT_YUV420P_NV21 </td>
                                     <td>pPlane[0] even Y lines, byte offset to next even line is ptSprite::stride*2 <br>
                                         pPlane[1] odd  Y lines, byte offset to next odd  line is ptSprite::stride*2 <br>
                                         pPlane[2] UV interleaved lines for NV12, 
                                                   VU interleaved lines for NV21, byte stride is in ptSprite::strideUV <br>
                                         <br>
                                         strideUV specifies the stride of the combined U/V plane.
                                     </td></tr>
                                 <tr><td>other planar YUV formats</td>
                                     <td>pPlane[0] Y lines, byte stride is ptSprite::stride <br>
                                         pPlane[1] U/V lines, byte stride is ptSprite::strideUV <br>
                                         pPlane[2] V/U lines, byte stride is ptSprite::strideUV
                                     </td></tr>
                                 </table>
                            */
        bbU8    data[4];    /**< Pixel data following. Pitch must align rows on 4 byte boundaries, and unused bits to the left must be 0. */
    };

protected:
    void Destroy(); //!< Release any buffers
    bbERR Convert_Pal2Pal(ptSprite* pDst) const;
    bbERR Convert_YUV2Pal(ptSprite* pDst) const;
    bbERR Convert_YUV2RGB(ptSprite* pDst) const;
    bbERR Convert_RGB2RGB(ptSprite* pDst) const;

public:
    ptSprite() { bbMemClear(this, sizeof(*this)); }
    ~ptSprite() { Destroy(); }

    inline ptCOLFMT GetColFmt() const { return (ptCOLFMT)colfmt; }
    inline ptENDIAN GetEndian() const { return (ptENDIAN)endian; }
    inline ptBITORDER GetBitOrder() const { return (ptBITORDER)bitorder; }
    inline bbU32 GetWidth() const { return width; }        //!< Get width in pixels.
    inline bbU32 GetHeight() const { return height; }      //!< Get height in pixels.
    inline bbU32 GetStride() const { return stride; }      //!< Get stride of first plane in bytes. Stride of a rasterline in bytes, for planar formats this is the stride in the 1st plane.
    inline bbU32 GetStrideUV() const { return strideUV; }  //!< Get stride of a U,V rasterline in bytes, for planar YUV formats only.
    inline ptPal* GetPal() const { return pPal; }
    inline ptYUV2RGB* GetYUV2RGB() const { return poYUV2RGB; }

    /** Get pointer to start of pixeldata, read-only access.
        @return Pointer to start of data in first plane.
    */
    inline const bbU8* GetData() const { return pData; }

    /** Get pointer to start of pixeldata, write access.
        @return Pointer to start of data in first plane.
    */
    inline bbU8* GetData() { return pData; }

    inline const bbU8* GetPlane(bbUINT plane) const { return pPlane[plane]; }
    inline bbU8* GetPlane(bbUINT plane) { return pPlane[plane]; }

    inline const bbU8* GetScanLine(bbUINT i) const { return pData + stride*i; }
    inline bbU8* GetScanLine(bbUINT i) { return pData + stride*i; }

    /** Release any buffers and clear instance to a 0-sized sprite. */
    void Clear();

    /** Creates a sprite on external pixel buffer.
        Constructs a sprite with the given width, height and format, that uses an existing memory buffer.
        The width and height must be specified in pixels, strides in bytes.

        There are no alignment requirement of the pixel data in the buffer.
        The buffer must remain valid throughout the life of the ptSprite.
        The sprite instance does not delete the buffer at destruction.

        If format is an palette or YUV colour format, the palette or YUV2RGB matrix is uninitialized and must be
        subsequently be set with ptSprite::SetPal() or ptSprite::SetYUV2RGB() before the image is used.

        For chunky formats (including chunky YUV formats), pPlanes[0] is the only pointer to the pixel data.
        For planar palette modes, pPlanes[0-7] contain pointer to the bitplanes, number of used planes depends on the format.
        For planar YUV modes, pPlanes[0-2] contain pointers to the Y,U,V plane or Y,U/V plane respectively.
        (Note for YUV420-type formats: Although only pPlanes[0-2] need to be given for the Create() call, ptSprite internally manages
        even and odd Y lines separately, i.e. ptSprite::pPlanes[0-3] will be used, see ptSprite::pPlane.)

        @param pPlanes Array of pointers to data for each plane. Array size depends on colour format.
    */
    void Create(bbU8** pPlanes, bbU32 width, bbU32 height, bbU32 stride, bbU32 strideUV, ptCOLFMT fmt, ptENDIAN endian, ptBITORDER bitorder);

    /** Creates a sprite on external pixel buffer, overload for planar modes.
        @see ptSprite::Create()
    */
    void Create(bbU8* pData, bbU32 width, bbU32 height, bbU32 stride, ptCOLFMT fmt, ptENDIAN endian=ptENDIAN_LE, ptBITORDER bitorder=ptBITORDER_LSBLEFT);

    /** Set external palette object.
        The palette must remain valid throughout the life of the ptSprite, and will not be freed.
    */
    void SetPal(const ptPal* pPal) { this->pPal = (ptPal*)pPal; }

    /** Set external YUV2RGB conversion matrix object.
        The matrix must remain valid throughout the life of the ptSprite, and will not be freed.
    */
    void SetYUV2RGB(const ptYUV2RGB* pMatrix) { this->poYUV2RGB = (ptYUV2RGB*)pMatrix; }

    /** Convert sprite to target format.
        The target sprite must be initialized to width and height identical with source sprite.
        All other parameters (colour format, endianess, bitorder, colour lookup) will be respected
        during the conversion process.
        @param pDst Target sprite container to be filled with converted data
    */
    bbERR Convert(ptSprite* pDst) const;
};

#define ptSIZEOF_SPRITEHDR (sizeof(ptSprite)-sizeof(void*)*8)

/** Create a sprite object of given properties.
    @param width Width in pixels
    @param width Height in pixels
    @param depth Log2 of colour bit depth, valid values or 0,1,2,3,4,5.
    @return Pointer to C-heap block containing new sprite.
            Control of this object is passed to the caller.
            Returns NULL on failure, check bbgErr.
*/
ptSprite* ptSpriteCreate( bbUINT const width, bbUINT const height, bbUINT const depth);

/** Convert a sprite.
    @param pSrcSprite Source sprite
    @param depth Log2 of bit depth of destination sprite.
    @return Pointer to C-heap block containing new sprite.
            Control of this object is passed to the caller.
            Returns NULL on failure, check bbgErr.
*/
ptSprite* ptSpriteConvert( const ptSprite* const pSrcSprite, bbUINT const depth);

#endif /* ptSPRITE_H_ */
