#pragma once

#include "Common/Meta.h"
#include <OpenCL/cl.hpp>

namespace CLCommon {

struct CLCommon {
	//whether we want to request the GPU or CPU.
	//only used by getDevice() which is called at construction
	bool useGPU;	
	
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commands;

	CLCommon(bool useGPU_ = true);
	virtual cl::Platform getPlatform();
	virtual cl::Device getDevice(cl::Platform platform);
};

//helper functions


template<int index>
struct SetArg {
	template<typename... Args>
	static bool exec(cl::Kernel kernel, Args... args) {
		kernel.setArg(index, std::get<index>(std::make_tuple(args...)));
		return false;
	}
};

//helper functions
template<typename... ArgList>
inline void setArgs(cl::Kernel kernel, ArgList&&... args) {
	typedef TypeVector<ArgList...> Args;	
	ForLoop<0, Args::size, SetArg>::exec(kernel, std::forward<ArgList>(args)...);
}

};

