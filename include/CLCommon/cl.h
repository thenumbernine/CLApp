#ifdef PLATFORM_osx
#include <OpenCL/cl2.h>
#elif PLATFORM_msvc
#include "CL/cl2.h"
#else
#include <CL/cl2.h>
#endif
