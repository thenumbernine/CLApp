#if PLATFORM_OSX
#include <OpenCL/cl2.h>
#elif PLATFORM_MSVC
#include "CL/cl2.h"
#else
#include <CL/cl2.h>
#endif
