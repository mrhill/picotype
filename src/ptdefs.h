#ifndef ptDEFS_H_
#define ptDEFS_H_

/* ptconfig.h version picotype expects.

   The header ptconfig.h usually resides in the application project
   directory to enable project specific configuration. This define
   specifies the expected ptconfig.h version.
*/
#define ptNEEDCONFIGVER 4

#include <babel/defs.h> /* for auto config */
#include "ptconfig.h"   /* include user config, will search in
                           local app project dir or include/picotype in this order */

#if ptCONFIGVER != ptNEEDCONFIGVER
#error ptconfig.h version mismatch
#endif

/** picotype error codes. */
enum ptERR
{
    ptUK = bbEBASE_PT,
    ptEFONTWININIT,     /**< Error code, error initializing windows font for ptFont_win */
};

/** Rectangle descriptor. */
typedef struct
{
    bbS32 left;     //!< Left border, inclusive
    bbS32 top;      //!< Left border, inclusive
    bbS32 right;    //!< Left border, exclusive unless stated otherwise
    bbS32 bottom;   //!< Left border, exclusive unless stated otherwise

} ptRect;

struct ptPal;
class ptYUV2RGB;

/** Index size of anti-alias lookup table used by ptGC8.
    Must be a power of two. Space needed for the table
    is ptGCAASIZE*ptGCAASIZE*8 bytes.
*/
#define ptGCAASIZE 16

/** Log2 of #ptGCAASIZE. */
#define ptGCAASIZE_LOG2 4

/** Fix-point precission for AA calculations. @internal */
#define ptGCAAPREC 3

/** Colour space types */
enum ptCOLTYPE
{
    ptCOLTYPE_PAL,  //!< Palette indexed
    ptCOLTYPE_RGB,  //!< RGB colour space
    ptCOLTYPE_YUV,  //!< YUV colour space
    ptCOLTYPE_GREY, //!< Grey scale
    ptCOLTYPECOUNT
};

/** Colour format IDs. */
enum ptCOLFMT
{
    ptCOLFMT_1BPP = 0,
    ptCOLFMT_2BPP,
    ptCOLFMT_4BPP,
    ptCOLFMT_8BPP,
    ptCOLFMT_2BPPP,
    ptCOLFMT_3BPPP,
    ptCOLFMT_4BPPP,
    ptCOLFMT_5BPPP,
    ptCOLFMT_6BPPP,
    ptCOLFMT_7BPPP,
    ptCOLFMT_8BPPP,
    ptCOLFMT_RGB565,
    ptCOLFMT_RGBA1555,
    ptCOLFMT_RGBA4444,
    ptCOLFMT_RGB888,
    ptCOLFMT_BGR888,
    ptCOLFMT_RGBA8888,
    ptCOLFMT_BGRA8888,
    ptCOLFMT_YUV420P,       // these 6 must be in this order:  // only YUV below this point
    ptCOLFMT_YUV420P_YV12,  // - even/odd offset is UV/VU order
    ptCOLFMT_YUV420P_IMC3,  // - IMCx need to be in a group
    ptCOLFMT_YUV420P_IMC1,  //
    ptCOLFMT_YUV420P_IMC4,  //
    ptCOLFMT_YUV420P_IMC2,  //
    ptCOLFMT_YUV420P_NV12,//Y,UV
    ptCOLFMT_YUV420P_NV21,//Y,VU
    ptCOLFMT_YUV420P_12,
    ptCOLFMT_YUV420P_16,
    ptCOLFMT_YUYV,
    ptCOLFMT_UYVY,
    ptCOLFMT_YVYU,
    ptCOLFMT_VYUY,
    ptCOLFMT_YUV422P,
    ptCOLFMT_YUV422RP,
    ptCOLFMT_YUV444,
    ptCOLFMT_YUV444P,
    ptCOLFMT_AYUV,
    ptCOLFMTCOUNT
};

/** Colour format description. */
struct ptColFmtInfo
{
    bbU8 bpp;           //!< Bits per pixel
    bbU8 widthalign;    //!< Alignment requirement for pixel width
    bbU8 accessunit;    //!< Bytes per access unit
    bbU8 pixalign;      //!< Alignment requirement for byte access to pixel data, must be power of 2-1
    bbU8 flags;         //!< Flag bitmask see ptCOLFMTFLAG, top 4 bits is pixels per byte - 1
    bbU8 PlaneCount;    //!< Number of planes
    bbU8 PlaneShiftH;   //!< Horizontal downsampling for subplanes (planes other than plane 0)
    bbU8 PlaneShiftV;   //!< Vertical downsampling for subplanes (planes other than plane 0)
};

/** ptColFmtInfo::flags bits. */
enum ptCOLFMTFLAG
{
    ptCOLFMTFLAG_PALETTE  = 0x1,    //!< Colour format is palette indexed
    ptCOLFMTFLAG_RGB      = 0x2,    //!< Colour format has RGB pixel data
    ptCOLFMTFLAG_YUV      = 0x4,    //!< Colour format has YUV pixel data
    ptCOLFMTFLAG_ALPHA    = 0x8,    //!< Colour format has alpha channel
    ptCOLFMTFLAG_SWAPUV   = 0x10,   //!< Internal use: swap VU planes to use UV rendering code
    ptCOLFMTFLAG_PPBSHIFT = 5,      //!< Bitposition of PPB field in ptColFmtInfo::flags
    ptCOLFMTFLAG_PPBMASK  = 7,      //!< Mask to mask PPB from ptColFmtInfo::flags
};

#define ptCOLFMTINFO \
    {/*ptCOLFMT_1BPP         */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 1, 0, 0},\
    {/*ptCOLFMT_2BPP         */  2, 4, 1, 0, ptCOLFMTFLAG_PALETTE|(3<<ptCOLFMTFLAG_PPBSHIFT), 1, 0, 0},\
    {/*ptCOLFMT_4BPP         */  4, 2, 1, 0, ptCOLFMTFLAG_PALETTE|(1<<ptCOLFMTFLAG_PPBSHIFT), 1, 0, 0},\
    {/*ptCOLFMT_8BPP         */  8, 1, 1, 0, ptCOLFMTFLAG_PALETTE|(0<<ptCOLFMTFLAG_PPBSHIFT), 1, 0, 0},\
    {/*ptCOLFMT_2BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 2, 0, 0},\
    {/*ptCOLFMT_3BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 3, 0, 0},\
    {/*ptCOLFMT_4BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 4, 0, 0},\
    {/*ptCOLFMT_5BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 5, 0, 0},\
    {/*ptCOLFMT_6BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 6, 0, 0},\
    {/*ptCOLFMT_7BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 7, 0, 0},\
    {/*ptCOLFMT_8BPPP        */  1, 8, 1, 0, ptCOLFMTFLAG_PALETTE|(7<<ptCOLFMTFLAG_PPBSHIFT), 8, 0, 0},\
    {/*ptCOLFMT_RGB565       */ 16, 1, 2, 0, ptCOLFMTFLAG_RGB,                    1, 0, 0},\
    {/*ptCOLFMT_RGBA1555     */ 16, 1, 2, 0, ptCOLFMTFLAG_RGB|ptCOLFMTFLAG_ALPHA, 1, 0, 0},\
    {/*ptCOLFMT_RGBA4444     */ 16, 1, 2, 0, ptCOLFMTFLAG_RGB|ptCOLFMTFLAG_ALPHA, 1, 0, 0},\
    {/*ptCOLFMT_RGB888       */ 24, 1, 1, 0, ptCOLFMTFLAG_RGB,                    1, 0, 0},\
    {/*ptCOLFMT_BGR888       */ 24, 1, 1, 0, ptCOLFMTFLAG_RGB,                    1, 0, 0},\
    {/*ptCOLFMT_RGBA8888     */ 32, 1, 4, 0, ptCOLFMTFLAG_RGB|ptCOLFMTFLAG_ALPHA, 1, 0, 0},\
    {/*ptCOLFMT_BGRA8888     */ 32, 1, 4, 0, ptCOLFMTFLAG_RGB|ptCOLFMTFLAG_ALPHA, 1, 0, 0},\
    {/*ptCOLFMT_YUV420P      */  8, 2, 1, 1, ptCOLFMTFLAG_YUV,                     3, 1, 1},\
    {/*ptCOLFMT_YUV420P_YV12 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 3, 1, 1},\
    {/*ptCOLFMT_YUV420P_IMC3 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV,                     3, 0, 1}, /*PlaneShiftH is 0 (UV stride is equal to Y stride)*/\
    {/*ptCOLFMT_YUV420P_IMC1 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 3, 0, 1},\
    {/*ptCOLFMT_YUV420P_IMC4 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV,                     2, 0, 1}, /*PlaneShiftH is 0, plane count 2 (interleaved UV is 1 plane)*/\
    {/*ptCOLFMT_YUV420P_IMC2 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 2, 0, 1},\
    {/*ptCOLFMT_YUV420P_NV12 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV,                     2, 0, 1}, /**/\
    {/*ptCOLFMT_YUV420P_NV21 */  8, 2, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 2, 0, 1},\
    {/*ptCOLFMT_YUV420P_12   */ 16, 2, 2, 1, ptCOLFMTFLAG_YUV,                     3, 1, 1},\
    {/*ptCOLFMT_YUV420P_16   */ 16, 2, 2, 1, ptCOLFMTFLAG_YUV,                     3, 1, 1},\
    {/*ptCOLFMT_YUYV         */ 16, 4, 1, 1, ptCOLFMTFLAG_YUV,                     1, 0, 0},\
    {/*ptCOLFMT_UYVY         */ 16, 4, 1, 1, ptCOLFMTFLAG_YUV,                     1, 0, 0},\
    {/*ptCOLFMT_YVYU         */ 16, 4, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 1, 0, 0},\
    {/*ptCOLFMT_VYUY         */ 16, 4, 1, 1, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_SWAPUV, 1, 0, 0},\
    {/*ptCOLFMT_YUV422P      */  8, 2, 1, 1, ptCOLFMTFLAG_YUV,                     3, 1, 0},\
    {/*ptCOLFMT_YUV422RP     */  8, 1, 1, 1, ptCOLFMTFLAG_YUV,                     3, 0, 1},\
    {/*ptCOLFMT_YUV444       */ 24, 1, 1, 0, ptCOLFMTFLAG_YUV,                     1, 0, 0},\
    {/*ptCOLFMT_YUV444P      */  8, 1, 1, 0, ptCOLFMTFLAG_YUV,                     3, 0, 0},\
    {/*ptCOLFMT_AYUV         */ 32, 1, 4, 0, ptCOLFMTFLAG_YUV|ptCOLFMTFLAG_ALPHA,  1, 0, 0}

extern ptColFmtInfo ptgColFmtInfo[ptCOLFMTCOUNT];

ptCOLTYPE ptColFmtGetType(ptCOLFMT fmt);

/** Test if colour format ID is a YUV format.
    @param (ptCOLFMT) Colour format ID
    @return true or false
*/
#define ptColFmtIsYUV(colfmt) ((bbUINT)colfmt>=ptCOLFMT_YUV420P)

/** Test if colour format ID is planar YUV420 format.
    @param (ptCOLFMT) Colour format ID
    @return true or false
*/
#define ptColFmtIsYUV420(colfmt) (((bbUINT)colfmt-ptCOLFMT_YUV420P)<=ptCOLFMT_YUV420P_IMC2)

#define ptColFmtIsIndexed(colfmt) ((ptgColFmtInfo[colfmt].flags & ptCOLFMTFLAG_PALETTE) != 0)

/** Test if colour format ID is has alpha channel.
    @param (ptCOLFMT) Colour format ID
    @return true if alpha, false otherwise
*/
#define ptColFmtHasAlpha(colfmt) ((ptgColFmtInfo[colfmt].flags & ptCOLFMTFLAG_ALPHA) != 0)

/** Test if colour format is sensible to bitorder.
    @param (ptCOLFMT) Colour format ID
    @return true if bitorder matters, false otherwise
    @see ptBITORDER
*/
#define ptColFmtHasBitorder(colfmt) ((ptgColFmtInfo[colfmt].flags & (ptCOLFMTFLAG_PPBMASK<<ptCOLFMTFLAG_PPBSHIFT)) != 0)

/** Test if colour format is sensible to byte endianess.
    @param (ptCOLFMT) Colour format ID
    @return true if endianess matters, false otherwise
*/
#define ptColFmtHasEndianess(colfmt) (ptgColFmtInfo[colfmt].accessunit > 1)

/** Get pixels per byte (minus 1) for colour format.
    This call only works for colour formats, where a byte holds one or more complete pixels
    @return PPB -1
*/
#define ptColFmtGetPPB(colfmt) ((bbUINT)ptgColFmtInfo[colfmt].flags >> ptCOLFMTFLAG_PPBSHIFT)

#define ptCOLFMTNAMES \
    bbT("1 bpp"),\
    bbT("2 bpp"),\
    bbT("4 bpp"),\
    bbT("8 bpp"),\
    bbT("2 bpp planar"),\
    bbT("3 bpp planar"),\
    bbT("4 bpp planar"),\
    bbT("5 bpp planar"),\
    bbT("6 bpp planar"),\
    bbT("7 bpp planar"),\
    bbT("8 bpp planar"),\
    bbT("RGB565"),\
    bbT("RGBA1555"),\
    bbT("RGBA4444"),\
    bbT("RGB888"),\
    bbT("BGR888"),\
    bbT("RGBA8888"),\
    bbT("BGRA8888"),\
    bbT("YUV420 planar I420"),\
    bbT("YVU420 planar YV12"),\
    bbT("YUV420 planar IMC3"),\
    bbT("YVU420 planar IMC1"),\
    bbT("YUV420 planar IMC4"),\
    bbT("YVU420 planar IMC2"),\
    bbT("YVU420 planar NV12"),\
    bbT("YVU420 planar NV21"),\
    bbT("YVU420 planar 12-bit"),\
    bbT("YVU420 planar 16-bit"),\
    bbT("YUV422 YUYV"),\
    bbT("YUV422 UYVY"),\
    bbT("YUV422 YVYU"),\
    bbT("YUV422 VYUY"),\
    bbT("YUV422 planar YV16"),\
    bbT("YUV422R planar"),\
    bbT("YUV444"),\
    bbT("YUV444 planar"),\
    bbT("AYUV")

/** Maximum string length for names in ptCOLFMTNAMES array. */
#define ptCOLFMTNAMEMAXLEN 24

/** Index of FourCC code in colour format name, or 0 if none. */
#define ptCOLFMTFOURCCIDX 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,14,14,14,14,14,14,14,0,0,7,7,7,7,14,0,0,0,0

/** YUV to RGB conversion matrix IDs. */
enum ptYUV2RGBID
{
    ptYUV2RGBID_JFIF = 0,
    ptYUV2RGBID_YCBCR_SDTV,
    ptYUV2RGBID_YCBCR_HDTV,
    ptYUV2RGBID_YIQ,
    ptYUV2RGBID_PHOTOYCC,
    ptYUV2RGBIDCOUNT
};

#define ptYUV2RGBNAMES \
    bbT("JFIF"), \
    bbT("YCbCr SDTV (ITU-R BT.601)"), \
    bbT("YCbCr HDTV"), \
    bbT("YIQ NTSC"), \
    bbT("PhotoYCC")

#define ptYUV2RGBMATRIX_JFIF \
   0, -128, -128, \
   (int)(1.000 * (1<<10)), (int)(0.000 * (1<<10)), (int)(1.402 * (1<<10)), \
   (int)(1.000 * (1<<10)), (int)(0.34414*(1<<10)), (int)(0.71414*(1<<10)), \
   (int)(1.000 * (1<<10)), (int)(1.772 * (1<<10)), (int)(0.000 * (1<<10))

#define ptYUV2RGBMATRIX_YCBCR_SDTV \
   -16, -128, -128, \
   (int)(1.164 * (1<<10)), (int)(0.000 * (1<<10)), (int)(1.596 * (1<<10)), \
   (int)(1.164 * (1<<10)), (int)(-0.391* (1<<10)), (int)(-0.813* (1<<10)), \
   (int)(1.164 * (1<<10)), (int)(2.018 * (1<<10)), (int)(0.000 * (1<<10))

#define ptYUV2RGBMATRIX_YCBCR_HDTV \
   -16, -128, -128, \
   (int)(1.164 * (1<<10)), (int)(0.000 * (1<<10)), (int)(1.793 * (1<<10)), \
   (int)(1.164 * (1<<10)), (int)(-0.213* (1<<10)), (int)(-0.534* (1<<10)), \
   (int)(1.164 * (1<<10)), (int)(2.115 * (1<<10)), (int)(0.000 * (1<<10))

#define ptYUV2RGBMATRIX_YIQ \
   -16, -128, -128, \
   (int)(1.000 * (1<<10)), (int)(0.9563 * (1<<10)), (int)(0.621 * (1<<10)), \
   (int)(1.000 * (1<<10)), (int)(-0.2721* (1<<10)), (int)(-0.6474*(1<<10)), \
   (int)(1.000 * (1<<10)), (int)(-1.107 * (1<<10)), (int)(1.7046* (1<<10))

#define ptYUV2RGBMATRIX_PHOTOYCC \
   0, -156, -137, \
   (int)(0.981 * (1<<10)), (int)(0.000 * (1<<10)), (int)(1.315 * (1<<10)), \
   (int)(0.981 * (1<<10)), (int)(-0.311* (1<<10)), (int)(-0.699* (1<<10)), \
   (int)(0.981 * (1<<10)), (int)(1.601 * (1<<10)), (int)(0.000 * (1<<10))

/** Byte endianess modifier for colour format.
    Valid only for colour formats with 16/32/64 bit atomic access units.
*/
enum ptENDIAN
{
    ptENDIAN_LE = 0,  //!< Little endian
    ptENDIAN_BE,      //!< Big endian
#if bbCPUE == bbCPUE_LE
    ptENDIAN_NATIVE = ptENDIAN_LE,  //!< Native endianess
#else
    ptLENDIAN_NATIVE = ptENDIAN_BE, //!< Native endianess
#endif
};

/** Bitorder modifier for colour format.
    Valid only for colour formats with more than 1 pixels per byte, or
    planar palette colour formats.
*/
enum ptBITORDER
{
    ptBITORDER_LSBLEFT = 0, //!< Show LSB left
    ptBITORDER_MSBLEFT,     //!< Show MSB left
};

#endif /* ptDEFS_H_ */

