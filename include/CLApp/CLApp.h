#pragma once

#include "GLApp/GLApp.h"
#include <OpenCL/cl.hpp>

struct CLApp : public GLApp {
	bool useGPU;	//whether we want to request the GPU or CPU.  must be set prior to init()
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commands;

	CLApp();
	virtual cl::Platform getPlatform();
	virtual cl::Device getDevice(cl::Platform platform);
	
	virtual void init();
};

