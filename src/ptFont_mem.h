#ifndef ptFONT_MEM_H_
#define ptFONT_MEM_H_

/** @file
*/

#include "ptFont.h"
#include "babel/file.h"

/** Simple font implementation, which loads all characters to memory.
    This font implementation supports loading bitmap fonts from the
    following formats:
     - ZapFont
*/
class ptFont_mem : public ptFont
{
private:
    /** Initialize font container from a ZapFont file.
        Container must be cleared.
        The container remains cleared on failure.

        @param fh Handle to opened file.
        @return Replacement for unknown CP on success, or (bbCHARCP)-1 on failure
    */
    #if (ptUSE_FONTZAP == 1) || defined(DOXYGEN)
    bbCHARCP LoadZapFont(bbFILEH const fh);
    #endif

    /** Initialize font container from a BBC font file.
        Container must be cleared.
        The container remains cleared on failure.

        @param fh Handle to opened file.
        @return Replacement for unknown CP on success, or (bbCHARCP)-1 on failure
    */
    #if (ptUSE_FONTBBC == 1) || defined(DOXYGEN)
    bbCHARCP LoadBBCFont(bbFILEH const fh);
    #endif

public:
    ptFont_mem();

    /** Initialize font container from a file.
        Will detect file format if supported

        Will clear previous font handled by the container.
        The container remains cleared on failure.

        @param pFilename Path to font file
        @return bbEOK on success, or value of #bbgErr on failure.
    */
    bbERR Load(const bbCHAR* const pFilename);
};

#endif /* ptFONT_MEM_H_ */
