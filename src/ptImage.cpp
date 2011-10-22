#include "ptImage.h"
#include "babel/mem.h"

bbERR ptImage::SelectImage(const bbUINT index)
{
    mCurImage = index;
    return bbEOK;
}

void ptImage::ReadEnd()
{
}

ptSprite* ptImageGetSprite( ptImage* const pImage)
{
    // get input image info and prepare reading

    ptImageInfo info;
    if ( (pImage->GetInfo( &info) != bbEOK) ||
         (pImage->ReadStart() != bbEOK) )
    {
        return NULL;
    }

    // create output sprite
    ptSprite* const pSprite = ptSpriteCreate( info.width, info.height, info.depth);
    if (!pSprite) return pSprite;

    // copy pixel data

    bbU8* pDst = pSprite->data;

    for (bbUINT y=0; y<info.height; ++y)
    {
        if (pImage->ReadNextLine(pDst) != bbEOK) goto ptImage_GetSprite_err;
        pDst += pSprite->stride;
    }

    pImage->ReadEnd();

    return pSprite;

    ptImage_GetSprite_err:

    bbMemFree( pSprite);

    return NULL;
}
