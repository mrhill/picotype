#include "ptImage_jpeglib.h"

#if ptUSE_JPEGLIB == 1

/* --- error call back for jpeglib ------------------------- */

/*
METHODDEF(void) _error_exit (j_common_ptr cinfo) 
{
    struct ptImage_jpeglib_error_mgr* myerr = (struct ptImage_jpeglib_error_mgr*) cinfo->err;

    // Always display the message.
    // We could postpone this until after returning, if we chose.
    (*cinfo->err->output_message) (cinfo);

    // Return control to the setjmp point
    longjmp( myerr->setjmp_buffer, 1);
}
*/

/* --- read source data call back for jpeglib ------------------------- */

typedef struct
{
    struct jpeg_source_mgr pub; /* public fields */

    /*FILE * infile;*/  /* source stream */
    bbU8* pJPEG;

    JOCTET * buffer;    /* start of buffer */

} my_source_mgr;

METHODDEF(void) _init_source (j_decompress_ptr cinfo)
{
}

METHODDEF(BOOL) _fill_input_buffer (j_decompress_ptr cinfo)
{
    return TRUE;
}

METHODDEF(void) _skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr* const pSrcMgr = (my_source_mgr*) cinfo->src;

    if (num_bytes > 0)
    {
        while (num_bytes > (long) pSrcMgr->pub.bytes_in_buffer)
        {
            num_bytes -= (long) pSrcMgr->pub.bytes_in_buffer;

            /* (void) _fill_input_buffer(cinfo); */ /* complete jpeg already in memory */
        }
        pSrcMgr->pub.next_input_byte += (size_t) num_bytes;
        pSrcMgr->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

METHODDEF(void) _term_source (j_decompress_ptr cinfo)
{
}

/* --- ptImage_jpeglib implementation ------------------------- */

bbERR ptImage_jpeglib::Open( const bbU8* pJPEG, const bbUINT size)
{ 
    Close();

    mpJPEG = pJPEG;

    if (msJDecomp.src == NULL) /* first time for this JPEG object? */
    {     
        msJDecomp.src = (struct jpeg_source_mgr *)
                        (*msJDecomp.mem->alloc_small) ((j_common_ptr) &msJDecomp, JPOOL_PERMANENT, sizeof(my_source_mgr));
    }

    my_source_mgr* const pSrcMgr = (my_source_mgr*) msJDecomp.src;

    pSrcMgr->pub.init_source = _init_source;
    pSrcMgr->pub.fill_input_buffer = _fill_input_buffer;
    pSrcMgr->pub.skip_input_data = _skip_input_data;
    pSrcMgr->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    pSrcMgr->pub.term_source = _term_source;
    pSrcMgr->pJPEG = (bbU8*) pJPEG;

    /* src->pub.bytes_in_buffer = 0; */ /* forces fill_input_buffer on first read */
    pSrcMgr->pub.bytes_in_buffer = size;
    pSrcMgr->pub.next_input_byte = pJPEG; /* until buffer loaded */

    jpeg_read_header(&msJDecomp, TRUE);

    return bbEOK;
}

void ptImage_jpeglib::Close()
{
    if (mpJPEG)
    {
        mpJPEG = NULL;
    }
}

ptImage_jpeglib::ptImage_jpeglib()
{
    mpJPEG = NULL;

    msJDecomp.err = jpeg_std_error( &msJErrMgr.pub);
    jpeg_create_decompress( &msJDecomp);
}

ptImage_jpeglib::~ptImage_jpeglib()
{
    jpeg_destroy_decompress( &msJDecomp);
    Close();
}

bbERR ptImage_jpeglib::GetInfo(ptImageInfo* const pInfo)
{
    bbASSERT( mpJPEG);
    bbASSERT( msJDecomp.src);

    pInfo->width = msJDecomp.image_width;
    pInfo->height = msJDecomp.image_height;
    if (msJDecomp.num_components == 1)
    {
        pInfo->depth = 3;
        pInfo->bytesperline = msJDecomp.image_width;
    }
    else
    {
        pInfo->depth = 5;
        pInfo->bytesperline = msJDecomp.image_width << 2;
    }
    pInfo->flags = 0;

    return bbEOK;
}

bbCHAR* ptImage_jpeglib::GetName()
{
    return NULL;
}

bbERR ptImage_jpeglib::ReadStart()
{
    if (!jpeg_start_decompress( &msJDecomp))
    {
        return bbErrSet( bbEUK);
    }

    if ((msJDecomp.output_width != msJDecomp.image_width) ||
        (msJDecomp.output_components != msJDecomp.num_components))
        bbErrSet( bbEUK);

    return bbEOK;
}

bbERR ptImage_jpeglib::ReadNextLine( bbU8* const pBuf)
{
    JSAMPROW rows[1];
    rows[0] = pBuf;

    JDIMENSION readlines = jpeg_read_scanlines( &msJDecomp, &rows[0], 1);
    bbASSERT(readlines);

    if (msJDecomp.output_components == 3)
    {
        // adjust 24 bpp -> 32 bpp
        bbU32* pDst = (bbU32*) (pBuf + (msJDecomp.output_width << 2));
        bbU8*  pSrc = pBuf + (msJDecomp.output_width << 2) - msJDecomp.output_width;

        while (pSrc > pBuf)
        {
            pSrc -= 3;
            *(--pDst) = (bbU32)*pSrc | ((bbU32)*(pSrc+1)<<8) | ((bbU32)*(pSrc+2)<<16);
        }
    }

    return bbEOK;
}

void ptImage_jpeglib::ReadEnd()
{
    jpeg_abort_decompress( &msJDecomp);
}

#endif /* ptUSE_JPEGLIB == 1 */
