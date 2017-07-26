#ifdef PLATFORM_osx
#include <OpenCL/cl.h>
#elif PLATFORM_msvc
#include "CL/cl.h"
#else
#include <CL/cl.h>
#endif
