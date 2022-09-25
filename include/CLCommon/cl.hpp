#if defined(CL_HPP_TARGET_OPENCL_VERSION) && CL_HPP_TARGET_OPENCL_VERSION>=200

#if PLATFORM_OSX
#include <OpenCL/opencl.hpp>
#elif PLATFORM_MSVC
#include "CL/opencl.hpp"
#else
#include <CL/opencl.hpp>
#endif

#else	//CL_HPP_TARGET_OPENCL_VERSION

#if PLATFORM_OSX
#include <OpenCL/cl.hpp>
#elif PLATFORM_MSVC
#include "CL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#endif	//CL_HPP_TARGET_OPENCL_VERSION
