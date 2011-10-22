#ifndef ptPALMAN_H_
#define ptPALMAN_H_

#include <babel/defs.h>
#include <babel/Arr.h>
#include "ptdefs.h"
#include "ptPal.h"

bbDECLAREARRPTR(ptPal*, ptArrPPal);

#define ptPAL_MAXHANDLES 256

class ptPalMan
{
private:
    ptArrPPal   mPals;          /**< Registry for palette objects. */

    void DeletePal(ptPal* const pPal);
public:
    /** Init palette manager.
    */
    bbERR Init();

    /** Destroy palette manager. */
    void Destroy();

    inline bbUINT GetPalCount() const { return mPals.GetSize(); }

    inline ptPal* GetPal(bbUINT hPal) { return mPals[hPal]; }

    /** Add object to registry.
        @param pPal Palette object
        @return Runtime handle, or -1 on error
    */
    bbUINT AddPal(ptPal* const pPal);

    /** Get palette handle.
        @param Pointer to palette object
        @return ptPalMan local handle, or (bbUINT)-1 if object is unknown
    */
    bbUINT GetPalHandle(ptPal* const pPal);

    /** Destroy palette object
        @param pPal Palette object, can be NULL
    */
    void DestroyPal(ptPal* const pPal);

    /** Destroy palette object by handle
        @param hPal Palette handle, can be -1
    */
    void DestroyPalHandle(bbUINT const hPal);
};

/** Global palette manager instance. */
extern ptPalMan ptgPalMan;

#endif /* ptPALMAN_H_ */

