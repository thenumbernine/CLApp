DIST_FILENAME=CLApp
DIST_TYPE=lib

include ../Common/Base.mk
include ../GLApp/Include.mk
include ../TensorMath/Include.mk
include ../CLApp/Include.mk
# this is a library including libraries ... in which case their LDFLAGS might not be helpful

