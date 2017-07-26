#ifdef PLATFORM_osx
#include <OpenCL/cl.hpp>
#elif PLATFORM_msvc
#include "CL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif
