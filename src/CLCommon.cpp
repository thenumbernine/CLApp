#include "CLCommon/CLCommon.h"
#include "Tensor/Vector.h"
#include "Common/Exception.h"
#ifdef PLATFORM_linux	//ubuntu
#include "bits/stream_iterator.h"
#endif
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>

#if PLATFORM_osx
#include <OpenGL/CGLCurrent.h>
#endif
#if PLATFORM_msvc
#include <windows.h>
#include <GL/gl.h>
#endif

#define PAIR(x)	x, #x

namespace CLCommon {

CLCommon::CLCommon(
	bool useGPU_,
	bool verbose_,
	std::function<std::vector<cl::Device>::const_iterator(const std::vector<cl::Device>&)> pickDevice)
: useGPU(useGPU_)
, verbose(verbose_)
{
	platform = getPlatform();
	device = getDevice(platform, pickDevice);

#if PLATFORM_osx
	CGLContextObj kCGLContext = CGLGetCurrentContext();	// GL Context
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext); // Share Group
	cl_context_properties properties[] = {
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};
#endif
#if PLATFORM_msvc
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), // HGLRC handle
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), // HDC handle
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 
		0
	};	
#endif
#if PLATFORM_linux
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 
		0
	};	
#endif
	context = cl::Context({device}, properties);
	commands = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);
}

cl::Platform CLCommon::getPlatform() {
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (verbose) {
		for(cl::Platform &platform : platforms) {
			std::cout << "platform " << platform() << std::endl;
			std::vector<std::pair<cl_uint, const char *>> queries = {
				{PAIR(CL_PLATFORM_NAME)},
				{PAIR(CL_PLATFORM_VENDOR)},
				{PAIR(CL_PLATFORM_VERSION)},
				{PAIR(CL_PLATFORM_PROFILE)},
				{PAIR(CL_PLATFORM_EXTENSIONS)},
			};
			for (std::pair<cl_uint, const char *> &query : queries) {
				std::string param;
				platform.getInfo(query.first, &param);
				std::cout << query.second << ":\t" << param << std::endl;
			}
			std::cout << std::endl;
		}
	}

	return platforms[0];
}

struct DeviceParameterQuery {
	cl_uint param;
	const char *name;
	bool failed;
	DeviceParameterQuery(cl_uint param_, const char *name_) : param(param_), name(name_), failed(false) {}
	virtual void query(cl::Device device) = 0;
	std::string tostring() {
		if (failed) return "-failed-";
		return toStringType();
	}
	virtual std::string toStringType() = 0;
};

template<typename Type>
struct DeviceParameterQueryType : public DeviceParameterQuery {
	Type value;
	DeviceParameterQueryType(cl_uint param_, const char *name_) : DeviceParameterQuery(param_, name_), value(Type()) {}
	virtual void query(cl::Device device) {
		try {
			device.getInfo(param, &value);
		} catch (cl::Error &e) {
			failed = true;
		}
	}
	virtual std::string toStringType() {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}
};

template<>
struct DeviceParameterQueryType<char*> : public DeviceParameterQuery {
	std::string value;
	DeviceParameterQueryType(cl_uint param_, const char *name_) : DeviceParameterQuery(param_, name_) {}
	virtual void query(cl::Device device) {
		try {
			device.getInfo(param, &value);
		} catch (cl::Error &e) {
			failed = true;
		}
	}
	virtual std::string toStringType() { return value; }
};

//has to be a class separate of DeviceParameterQueryType because some types are used with both (cl_device_fp_config is typedef'd as a cl_uint)
template<typename Type>
struct DeviceParameterQueryEnumType : public DeviceParameterQuery {
	Type value;
	DeviceParameterQueryEnumType(cl_uint param_, const char *name_) : DeviceParameterQuery(param_, name_), value(Type()) {}
	virtual void query(cl::Device device) {
		try {
			device.getInfo(param, &value);
		} catch (cl::Error &e) {
			failed = true;
		}
	}
	virtual std::vector<std::pair<Type, const char *>> getFlags() = 0;
	virtual std::string toStringType() {
		std::vector<std::pair<Type, const char *>>flags = getFlags();
		Type copy = value;
		std::stringstream ss;
		for (std::pair<Type, const char*> &flag : flags) {
			if (copy & flag.first) {
				copy -= flag.first;
				ss << "\n\t" << flag.second;
			}
		}
		if (copy) {
			ss << "\n\textra flags: " << copy;
		}
		return ss.str();
	};
};

struct DeviceParameterQueryEnumType_cl_device_fp_config : public DeviceParameterQueryEnumType<cl_device_fp_config> {
	using DeviceParameterQueryEnumType::DeviceParameterQueryEnumType;
	virtual std::vector<std::pair<cl_device_fp_config, const char *>> getFlags() { 
		return std::vector<std::pair<cl_device_fp_config, const char *>>{
			{PAIR(CL_FP_DENORM)},
			{PAIR(CL_FP_INF_NAN)},
			{PAIR(CL_FP_ROUND_TO_NEAREST)},
			{PAIR(CL_FP_ROUND_TO_ZERO)},
			{PAIR(CL_FP_ROUND_TO_INF)},
			{PAIR(CL_FP_FMA)},
			{PAIR(CL_FP_SOFT_FLOAT)},
			{PAIR(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)},
		};
	}
};

struct DeviceParameterQueryEnumType_cl_device_exec_capabilities : public DeviceParameterQueryEnumType<cl_device_exec_capabilities> {
	using DeviceParameterQueryEnumType::DeviceParameterQueryEnumType;
	virtual std::vector<std::pair<cl_device_exec_capabilities, const char *>> getFlags() { 
		return std::vector<std::pair<cl_device_exec_capabilities, const char *>>{
			{PAIR(CL_EXEC_KERNEL)},
			{PAIR(CL_EXEC_NATIVE_KERNEL)},
		};
	}
};

//global scope ... er, static, whatever
std::vector<std::string> getExtensions(const cl::Device& device) {
	std::string extensionStr = device.getInfo<CL_DEVICE_EXTENSIONS>();
	extensionStr = std::string(extensionStr.c_str());	//remove trailing \0's *cough* AMD *cough*
	std::istringstream iss(extensionStr);
	
	std::vector<std::string> extensions;
	std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string>>(extensions));
	return extensions;
}

//likewise, static
std::vector<cl::Device>::const_iterator hasGLSharing(const std::vector<cl::Device>& devices) {
	return std::find_if(devices.begin(), devices.end(), [&](const cl::Device& device) -> bool {
		std::vector<std::string> extensions = getExtensions(device);
		return std::find_if(extensions.begin(), extensions.end(), [&](const std::string &s) -> bool {
			return s == std::string("cl_khr_gl_sharing") 	//i don't have
				|| s == std::string("cl_APPLE_gl_sharing");	//i do have!
		}) != extensions.end();
	});
}

cl::Device CLCommon::getDevice(
	cl::Platform platform,
	std::function<std::vector<cl::Device>::const_iterator(const std::vector<cl::Device>&)> pickDevice)
{
	std::vector<cl::Device> devices;
	platform.getDevices(useGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, &devices);

	std::vector<std::shared_ptr<DeviceParameterQuery>> deviceParameters = {
		std::make_shared<DeviceParameterQueryType<char*>>(PAIR(CL_DEVICE_NAME)),
		std::make_shared<DeviceParameterQueryType<char*>>(PAIR(CL_DEVICE_VENDOR)),
		std::make_shared<DeviceParameterQueryType<char*>>(PAIR(CL_DEVICE_VERSION)),
		std::make_shared<DeviceParameterQueryType<char*>>(PAIR(CL_DRIVER_VERSION)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_VENDOR_ID)),
		std::make_shared<DeviceParameterQueryType<cl_platform_id>>(PAIR(CL_DEVICE_PLATFORM)),
		std::make_shared<DeviceParameterQueryType<cl_bool>>(PAIR(CL_DEVICE_AVAILABLE)),
		std::make_shared<DeviceParameterQueryType<cl_bool>>(PAIR(CL_DEVICE_COMPILER_AVAILABLE)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_CLOCK_FREQUENCY)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_COMPUTE_UNITS)),
		std::make_shared<DeviceParameterQueryType<cl_device_type>>(PAIR(CL_DEVICE_TYPE)),
		std::make_shared<DeviceParameterQueryEnumType_cl_device_fp_config>(PAIR(CL_DEVICE_ADDRESS_BITS)),	//bitflags: 
		std::make_shared<DeviceParameterQueryEnumType_cl_device_fp_config>(PAIR(CL_DEVICE_HALF_FP_CONFIG)),
		std::make_shared<DeviceParameterQueryEnumType_cl_device_fp_config>(PAIR(CL_DEVICE_SINGLE_FP_CONFIG)),
		std::make_shared<DeviceParameterQueryType<cl_bool>>(PAIR(CL_DEVICE_ENDIAN_LITTLE)),
		std::make_shared<DeviceParameterQueryEnumType_cl_device_exec_capabilities>(PAIR(CL_DEVICE_EXECUTION_CAPABILITIES)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_ADDRESS_BITS)),
		std::make_shared<DeviceParameterQueryType<cl_ulong>>(PAIR(CL_DEVICE_GLOBAL_MEM_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_ulong>>(PAIR(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_device_mem_cache_type>>(PAIR(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE)),	//CL_NONE, CL_READ_ONLY_CACHE, and CL_READ_WRITE_CACHE.
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_ulong>>(PAIR(CL_DEVICE_LOCAL_MEM_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_device_local_mem_type>>(PAIR(CL_DEVICE_LOCAL_MEM_TYPE)),	//SRAM, or CL_GLOBAL
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MEM_BASE_ADDR_ALIGN)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_bool>>(PAIR(CL_DEVICE_IMAGE_SUPPORT)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_IMAGE2D_MAX_WIDTH)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_IMAGE2D_MAX_HEIGHT)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_IMAGE3D_MAX_WIDTH)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_IMAGE3D_MAX_HEIGHT)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_IMAGE3D_MAX_DEPTH)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_CONSTANT_ARGS)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_ulong>>(PAIR(CL_DEVICE_MAX_MEM_ALLOC_SIZE)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_MAX_PARAMETER_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_READ_IMAGE_ARGS)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_WRITE_IMAGE_ARGS)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_SAMPLERS)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_MAX_WORK_GROUP_SIZE)),
		std::make_shared<DeviceParameterQueryType<cl_uint>>(PAIR(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)),
		std::make_shared<DeviceParameterQueryType<Tensor::Vector<size_t,3>>>(PAIR(CL_DEVICE_MAX_WORK_ITEM_SIZES)),
		std::make_shared<DeviceParameterQueryType<char*>>(PAIR(CL_DEVICE_PROFILE)),
		std::make_shared<DeviceParameterQueryType<size_t>>(PAIR(CL_DEVICE_PROFILING_TIMER_RESOLUTION)),
		std::make_shared<DeviceParameterQueryType<cl_command_queue_properties>>(PAIR(CL_DEVICE_QUEUE_PROPERTIES)),
		//std::make_shared<DeviceParameterQueryType<char*>>(CL_DEVICE_EXTENSIONS, "extensions"),
	};

	if (verbose) {
		for (cl::Device device : devices) {
			std::cout << "device " << device() << std::endl;
			for (std::shared_ptr<DeviceParameterQuery> query : deviceParameters) {
				query->query(device);
				std::cout << query->name << ":\t" << query->tostring() << std::endl;
			}

			std::vector<std::string> extensions = getExtensions(device);
			std::cout << "CL_DEVICE_EXTENSIONS:" << std::endl;
			for (std::string &s : extensions) {
				std::cout << "\t" << s << std::endl;
			}
			std::cout << std::endl;
		}
	}

	//if pickDevice wasn't defined then just use the first
	std::vector<cl::Device>::const_iterator deviceIter = pickDevice ? pickDevice(devices) : devices.begin();
	
	if (deviceIter == devices.end()) throw Common::Exception() << "failed to find requested device";
	
	return *deviceIter;
}

};

