#ifdef PLATFORM_osx
#include <OpenCL/cl2.hpp>
#elif PLATFORM_msvc
#include "CL/cl2.hpp"
#else
#include <CL/cl2.hpp>
#endif
