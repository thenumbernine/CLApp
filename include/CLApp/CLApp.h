#pragma once

#include "GLApp/GLApp.h"
#include "Common/Meta.h"
#include <OpenCL/cl.hpp>

namespace CLApp {

struct CLApp : public ::GLApp::GLApp {
	typedef ::GLApp::GLApp Super;

	bool useGPU;	//whether we want to request the GPU or CPU.  must be set prior to init()
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commands;

	CLApp();
	virtual cl::Platform getPlatform();
	virtual cl::Device getDevice(cl::Platform platform);
	
	virtual void init();

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
	void setArgs(cl::Kernel kernel, ArgList&&... args) {
		typedef TypeVector<ArgList...> Args;	
		ForLoop<0, Args::size, SetArg>::exec(kernel, std::forward<ArgList>(args)...);
	}
};

};

