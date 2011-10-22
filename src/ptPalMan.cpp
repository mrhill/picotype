#include <babel/file.h>
#include <babel/str.h>
#include "ptPalMan.h"

ptPalMan ptgPalMan;

bbERR ptPalMan::Init()
{
    mPals.Init();
    return bbEOK;
}

void ptPalMan::Destroy()
{
#ifdef bbDEBUG
    for (bbUINT q=0; q<mPals.GetSize(); q++)
    {
        bbASSERT(mPals[q] == NULL);
    }
#endif

    mPals.Destroy();
}

bbUINT ptPalMan::AddPal(ptPal* const pPal)
{
    bbUINT i;

    for (i=0; i < (int)mPals.GetSize(); i++)
    {
        if (mPals[i] == NULL)
            goto ptPalMan_AddPal_foundfree;
    }

    if (!mPals.Grow(1))
        return (bbUINT)-1;

    mPals[i] = NULL;

    ptPalMan_AddPal_foundfree:
    bbASSERT(mPals[i] == NULL);

    mPals[i] = pPal;
    return i;
}

void ptPalMan::DeletePal(ptPal* const pPal)
{
    delete pPal;
}

bbUINT ptPalMan::GetPalHandle(ptPal* const pPal)
{
    int i = mPals.GetSize();
    while (i>0)
    {
        if (pPal == mPals[--i])
            return i;
    }
    return (bbUINT)-1;
}

void ptPalMan::DestroyPal(ptPal* const pPal)
{
    if (!pPal)
        return;

    bbUINT const i = GetPalHandle(pPal);

    if (i != (bbUINT)-1)
        mPals[i] = NULL;

    DeletePal(pPal);
}

void ptPalMan::DestroyPalHandle(bbUINT const hPal)
{
    if (hPal < mPals.GetSize())
        DestroyPal(mPals[hPal]);
}

