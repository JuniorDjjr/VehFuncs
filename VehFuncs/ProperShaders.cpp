#include "VehFuncsCommon.h"
#include "ProperShaders.h"

PS_Api properShaders = {};

void LoadProperShadersApi()
{
	if (!PS_ApiLoad(&properShaders))
	{
		lg << "Info: Proper Shaders isn't installed" << std::endl;
		return;
	}

	lg << "Info: Proper Shaders found (API version " << properShaders.version << ")" << std::endl;

	// The render pass queries only exist from API version 2 on. Testing the
	// pointer instead of the version number is what the API recommends, and it
	// is also what IsDuplicatedRenderCall() does, so an older Proper Shaders
	// just keeps the old behaviour instead of breaking.
	if (properShaders.IsMostCommonRenderCall == nullptr)
	{
		lg << "WARNING: This Proper Shaders is too old to tell us which render call is the scene pass, vehicles will be processed once per render pass." << std::endl;
	}
}

bool IsDuplicatedRenderCall()
{
	if (properShaders.IsMostCommonRenderCall == nullptr) return false;

	return properShaders.IsMostCommonRenderCall() == 0;
}
