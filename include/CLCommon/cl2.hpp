#ifdef PLATFORM_OSX
#include <OpenCL/cl2.hpp>
#elif PLATFORM_MSVC
#include "CL/cl2.hpp"
#else
#include <CL/cl2.hpp>
#endif
