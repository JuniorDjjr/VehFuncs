#include "VehFuncsCommon.h"
#include "CustomSeed.h"

extern list<CustomSeed> &getCustomSeedList()
{
	static list<CustomSeed> customSeedList;
	return customSeedList;
}

extern "C" int32_t __declspec(dllexport) Ext_GetCarSeed(CVehicle * vehicle) 
{
	ExtendedData &xdata = remInfo.Get(vehicle);

	lg << "Custom Seed: Get seed value: " << xdata.randomSeed << " for vehicle " << vehicle << "\n";

	return xdata.randomSeed;
}

extern "C" void __declspec(dllexport) Ext_SetCarSeed(CVehicle * vehicle, int seed) 
{
	CustomSeed * newCustomSeed = new CustomSeed((int)vehicle, seed);

	if (vehicle->m_nVehicleSubClass == VEHICLE_HELI) {
		lg << "Custom Seed: Currently VehFuncs (more specifically, plugin-sdk + SilentPatch) is incompatible with helicopters!!! \n";
	}
	else 
	{
		lg << "Custom Seed: New seed: " << newCustomSeed->seed << " for vehicle " << newCustomSeed->pvehicle << "\n";
		list<CustomSeed> &customSeedList = getCustomSeedList();
		customSeedList.push_back(*newCustomSeed);
	}
}
