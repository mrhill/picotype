#ifndef ptPAL_H_
#define ptPAL_H_

#include <babel/defs.h>
#include <babel/Arr.h>
#include "ptdefs.h"
#include "ptCol.h"

/** Flagbits for ptPal::mOpt. */
enum ptPALOPT
{
    ptPALOPT_EXTNAME = 0x1, //!< ptPal::mpName is managed externally
    ptPALOPT_EXTRGB  = 0x2, //!< ptPal::mpRGB is managed externally
    ptPALOPT_YUV     = 0x4  //!< ptPal::mpRGB contains ptYUVA values
};

/** Palette format IDs, see ptPal::mFormat. */
enum ptPALFMT
{
    ptPALFMT_NONE = 0,      //!< Palette was created using ptPalMan::CreatePal() or ptPal::CreatePredefined()
    ptPALFMT_NATIVE,        //!< Raw palette data in 0xAABBGGRR format, native endianess (ptPal internal format)
    ptPALFMT_RAWRGBA,       //!< Raw palette data in 0xAABBGGRR format, little endian
    ptPALFMT_RAWBGRA,       //!< Raw palette data in 0xAARRGGBB format, little endian
    ptPALFMT_RAWRGB,        //!< Raw palette data, little endian ordered RGB byte triples
    ptPALFMT_NATIVE_U64,    //!< Raw palette data in 0xAABBGGRR format, native endianess (ptPal internal format), array of bbU64 values
    ptPALFMT_ARCHIMEDES,    //!< Archimedes palette file
    ptPALFMTCOUNT
};

/** Predefined palette IDs. */
enum ptPALID
{
    ptPALID_GREY256 = 0,    //!< 256 entry greyscale palette
    ptPALID_RGB232,         //!< 256 entry R2G3B2 palette
    ptPALIDCOUNT
};

#define ptPALMAXCOL 256

/** RGB Palette container. */
struct ptPal
{
    bbU32*   mpRGB;      //!< Array of 0xAABBGGRR values
    bbU16    mColCount;  //!< Number of entries in mpRGB[]
    bbU8     mOpt;       //!< Flag bitmask, see ptPALOPT
    bbU8     mFormat;    //!< Palette format the object was created from, see ptPALFMT
    bbU32    mSyncPt;    //!< Sync point to enable caching
    bbCHAR*  mpName;     //!< Palette name, 0-terminated string, can be NULL

    static ptPal* CreatePredefined(ptPALID const id);
    static ptPal* Create(bbU8* pRGB , ptPALFMT srcfmt, bbUINT size, bbCHAR* const pName);

    ptPal(const bbU32* pExtRGB, bbUINT size, bbCHAR* pExtName);
    ptPal() { Init(); }
    ptPal(const ptPal& other);
    ~ptPal() { Destroy(); }
    ptPal& operator=(const ptPal& other);

    inline void Init()
    {
        bbMemClear(this, sizeof(ptPal));
    }

    bbERR Init(const ptPal& other);

    /** Delete object.
        Frees all memory associated with palette object
    */
    void Destroy() { Clear(); }

    /** Create palette object from RGB colour array.
        Must be destroyed with ptPal::Destroy or destructor later.
        @param pRGB Pointer to data, entries will be copied, can be NULL to create a palette object with unitialized RGB entries
        @param srcfmt Format of palette data in \a pRGB
        @param size Number of array entries or byte size in pPal[], interpretation depending on \a srcfmt
        @param pName Palette name, 0-terminated string, will be copied, can be NULL
    */
    bbERR Set(bbU8* pRGB , ptPALFMT srcfmt, bbUINT size, bbCHAR* const pName);

    void SetAlpha(bbU8 alpha, bbUINT colStart, bbUINT colEnd);

    bbERR SetPredefined(ptPALID const id);


    /** Empty palette container. */
    void Clear();

    bbERR SetColCount(bbUINT size)
    {
        bbASSERT((mOpt & ptPALOPT_EXTRGB) == 0);
        bbERR err = bbMemRealloc(size*4, (void**)&mpRGB);
        if (err == bbEOK)
        {
            mColCount = (bbU16)size;
        }
        return err;
    }

    const bbCHAR* GetName() const { return mpName; }

    /** Attach name string.
        @param pName Pointer to heap-block containing 0-terminated string. Can be NULL.
                     Ownership of block will be transfered to ptPal.
    */
    void AttachName(bbCHAR* pName)
    {
        if ((mOpt & ptPALOPT_EXTNAME) == 0)
            bbMemFree(mpName);
        mpName = pName;
        mOpt = mOpt &~ ptPALOPT_EXTNAME;
    }

    /** Load palette from file.
        @param pFileName Filename
    */
    bbERR Load(const bbCHAR* pFileName);

    /** Save palette to file.
        @param pPath 0-teminated file path string
        @param fmt File format ID
    */
    bbERR Save(const bbCHAR* pPath, ptPALFMT const fmt);

    bbERR Marshal(bbArrU8* pBuf);
    bbU8* Unmarshal(bbU8* pBuf);

    bbERR Load_Raw(bbU8* pBuf, bbU32 filesize);
    bbERR Load_Archimedes(bbU8* pBuf, bbU32 filesize);

    inline const bbU32* GetRGBA() const { return mpRGB; }
    inline bbUINT GetSize() const { return mColCount; }
    inline bbUINT GetColCount() const { return mColCount; }
    inline ptRGBA GetColRGBA(bbUINT idx) const { return mpRGB[idx]; }
    inline void   SetColRGBA(bbUINT idx, ptRGBA rgba) { mpRGB[idx]=rgba; mSyncPt++; }

    /** Find best matching index for supplied RGBA colour.
        @param rgba Colour to match
        @return Best matching index in palette
    */
    bbUINT MatchRGBA(ptRGBA rgba) const;

    /** Test if palette contains YUV values.
        @return !=0 if palette is YUV
    */
    inline int IsYUV() const { return mOpt & ptPALOPT_YUV; }

    /** Convert RGB palette to YUV.
        @param rgb2yuv Matrix to use
    */
    void ToYUV(const ptRGB2YUV& rgb2yuv);
};

#endif

