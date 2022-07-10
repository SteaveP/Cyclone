#include "Fault.h"
#include <assert.h>
#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include "windows.h"
#endif

//----------------------------------------------------------------------------
// FaultHandler
//----------------------------------------------------------------------------
void FaultHandler(const char* file, unsigned short line)
{
#if _WIN32
	// If you hit this line, it means one of the ASSERT macros failed.
    DebugBreak();
#endif

	assert(0);
}