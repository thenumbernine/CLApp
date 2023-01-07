#pragma once

#include "Common/Meta.h"
#include "Common/ForLoop.h"
#include "CLCommon/cl.hpp"
#include <functional>
#include <vector>

namespace CLCommon {

struct CLCommon {
	//whether we want to request the GPU or CPU.
	//only used by getDevice() which is called at construction
	bool useGPU = {};
	bool verbose = {};

	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commands;

	CLCommon(
		bool useGPU_ = true,
		bool verbose_ = false,
		//TODO instead of lambda, how about a pure virtual function that the implementation needs to overload?
		std::function<std::vector<cl::Device>::const_iterator(const std::vector<cl::Device> &)> pickDevice = std::function<std::vector<cl::Device>::const_iterator(const std::vector<cl::Device> &)>()
	);
	virtual ~CLCommon() {}
	
	virtual cl::Platform getPlatform();
	
	virtual cl::Device getDevice(
		cl::Platform platform,
		std::function<std::vector<cl::Device>::const_iterator(const std::vector<cl::Device> &)> pickDevice
	);
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
	using Args = std::tuple<ArgList...>;
	Common::ForLoop<0, std::tuple_size_v<Args>, SetArg>::exec(kernel, std::forward<ArgList>(args)...);
}

//useful
std::vector<std::string> getExtensions(const cl::Device& device);

//common pickDevice function...
std::vector<cl::Device>::const_iterator hasGLSharing(const std::vector<cl::Device>&);

}
