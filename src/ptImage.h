#ifndef ptIMAGE_H_
#define ptIMAGE_H_

#include <babel/defs.h>
#include "ptSprite.h"

/** Image information container for ptImage::GetInfo. */
typedef struct
{
    bbUINT width;       //!< Width in pixels
    bbUINT height;      //!< Height in pixels
    bbUINT bytesperline;//!< Length of a rasterline in bytes
    bbU8   depth;       //!< Log2 of colour bit depth. Valid values are 0,1,2,3,4,5, ie. 24 bpp must be stored as 32 bpp
    bbU8   flags;       //!< Flag bits
    bbU32  transcol;    //!< Dedicated colour that marks transparency

} ptImageInfo;

/** Flag bit for ptImageInfo::flags, image has a dedicated colour that marks transparency. */
#define ptIMAGEINFOFLAG_TRANSCOL 0x08
/** Flag bit for ptImageInfo::flags, image has binary transparency mask. */
#define ptIMAGEINFOFLAG_TRANSMASK 0x10
/** Flag bit for ptImageInfo::flags, image has alpha transparency mask. */
#define ptIMAGEINFOFLAG_ALPHAMASK 0x20

/** Interface to image formats.
    An ptImage instance can handle image containers with multiple images.
*/
struct ptImage
{
    bbU32 mStride;
    bbU32 mWidth;
    bbU32 mHeight;
    bbU32 mImageCount;
    
    bbUINT mCurImage; //!< Index of currently selected picture

    ptImage()
    {
        mCurImage = 0;
    }

    virtual ~ptImage() {}

    virtual bbERR Init(void* const pBuffer) = 0;
    virtual void Destroy() = 0;
    



    /** Select the active image for containers with multiple images.
        The default implementation simply stores \a index to ptImage::mCurImage.
        @param index Index of image in container.
        @return bbEOK on success, or error code on failure.
    */
    virtual bbERR SelectImage(const bbUINT index);

    /** Get image properties.
        @param pInfo Pointer to container for returning information
        @return bbEOK on success, or error code on failure
    */
    virtual bbERR GetInfo(ptImageInfo* const pInfo) = 0;

    /** Get name of currently selected image.
        @return Pointer to allocated C-Heap block containing 0-terminated string,
                or NULL on failure (see #bbgErr).
                Control of the heap block is passed to the caller.
    */
    virtual bbCHAR* GetName() = 0;

    /** Prepare reading of rasterline data width ptImage::ReadNextLine. 
        @return bbEOK on success, or error code on failure
    */
    virtual bbERR ReadStart() = 0;

    /** Get next rasterline from the currently selected image.
        @param pBuf Pointer to buffer for returning pixeldata.
                    Calle must ensure enough memory for a pixelline,
                    the amount of memory consumed by a pixelline is
                    ptImageInfo::width * (1<<ptImageInfo::depth) bits.
        @return bbEOK on success, or error code on failure
    */
    virtual bbERR ReadNextLine( bbU8* const pBuf) = 0;

    /** End reading of rasterline data width ptImage::ReadNextLine.
        Default implementation does nothing.
    */
    virtual void ReadEnd();
};

/** Create a sprite from the currently selected image of an ptImage.
    @param pImage Pointer to initialized image instance.
    @return Pointer to allocated C-Heap block containing sprite,
            or NULL on failure (see #bbgErr). Control of the
            heap block is passed to the caller.
*/
ptSprite* ptImageGetSprite( ptImage* const pImage);

#endif /* ptIMAGE_H_ */
