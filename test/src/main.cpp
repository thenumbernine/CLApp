#include "CLCommon/CLCommon.h"

int main() {
	CLCommon::CLCommon cl(true, true, 
		[&](
			const std::vector<cl::Device>& devices 
		) -> std::vector<cl::Device>::const_iterator {
			return devices.begin();	
		}
	);
}
