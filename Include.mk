CLAPP_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))

MACROS+=CL_HPP_TARGET_OPENCL_VERSION=200
MACROS+=CL_HPP_ENABLE_EXCEPTIONS
INCLUDE+=$(CLAPP_PATH)include
DYNAMIC_LIBS+=$(CLAPP_PATH)dist/$(PLATFORM)/$(BUILD)/libCLCommon$(LIB_SUFFIX)
LDFLAGS_osx+=-framework OpenCL
LIBS_linux+=OpenCL
