#ifndef ptIMAGE_JPEGLIB_H_
#define ptIMAGE_JPEGLIB_H_

/** @file
*/

#include "ptdefs.h"
#include "ptImage.h"

#if ptUSE_JPEGLIB == 1

extern "C" {
#include <jpeglib.h>

struct ptImage_jpeglib_error_mgr
{
    struct jpeg_error_mgr pub;  /* "public" fields */
    // add fields here
};

}

class ptImage_jpeglib : public ptImage
{
private:
    const bbU8* mpJPEG;
    struct jpeg_decompress_struct msJDecomp;
    struct ptImage_jpeglib_error_mgr msJErrMgr;

public:
    ptImage_jpeglib();

    /** Open JPEG from memory.
        @param pJPEG Pointer to JPEG in memory, must be available while class is used.
        @patam size Size of data in bytes
        @return bbEOK on success, or error code on failure.
    */
    bbERR Open( const bbU8* pJPEG, const bbUINT size);

    /** Close any attached JPEG data. */
    void Close();

    // ptImage interface implementation

    ~ptImage_jpeglib();
    bbERR GetInfo(ptImageInfo* const pInfo);
    bbCHAR* GetName();
    bbERR ReadStart();
    bbERR ReadNextLine( bbU8* const pBuf);
    void ReadEnd();
};

#endif /* ptUSE_JPEGLIB == 1 */

#endif /* ptIMAGE_JPEGLIB_H_ */
