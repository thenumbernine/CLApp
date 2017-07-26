#if defined(CL_HPP_TARGET_OPENCL_VERSION) && CL_HPP_TARGET_OPENCL_VERSION>=200

#ifdef PLATFORM_osx
#include <OpenCL/cl2.hpp>
#elif PLATFORM_msvc
#include "CL/cl2.hpp"
#else
#include <CL/cl2.hpp>
#endif

#else	//CL_HPP_TARGET_OPENCL_VERSION

#ifdef PLATFORM_osx
#include <OpenCL/cl.hpp>
#elif PLATFORM_msvc
#include "CL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#endif	//CL_HPP_TARGET_OPENCL_VERSION
