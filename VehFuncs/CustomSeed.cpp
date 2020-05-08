#include "VehFuncsCommon.h"
#include "CustomSeed.h"
#include "CTimer.h"

extern list<CustomSeed> &getCustomSeedList()
{
	static list<CustomSeed> customSeedList;
	return customSeedList;
}

extern "C" int32_t __declspec(dllexport) Ext_GetCarSeed(CVehicle * vehicle) 
{
	ExtendedData &xdata = xData.Get(vehicle);

	if (useLog) lg << "Custom Seed: Get seed value: " << xdata.randomSeed << " for vehicle " << vehicle << "\n";

	return xdata.randomSeed;
}

extern "C" void __declspec(dllexport) Ext_SetCarSeed(CVehicle * vehicle, int seed) 
{
	CustomSeed * newCustomSeed = new CustomSeed((int)vehicle, seed, CTimer::m_snTimeInMilliseconds + 1000);

	if (vehicle->m_nVehicleSubClass == VEHICLE_HELI) {
		if (useLog) lg << "Custom Seed: Currently VehFuncs (more specifically, plugin-sdk + SilentPatch) is incompatible with helicopters!!! \n";
	}
	else 
	{
		if (useLog) lg << "Custom Seed: New seed: " << newCustomSeed->seed << " for vehicle " << newCustomSeed->pvehicle << "\n";
		list<CustomSeed> &customSeedList = getCustomSeedList();
		customSeedList.push_back(*newCustomSeed);
	}
}

/*
The function name appears for crash dumps as modloader.log, and people mistakenly think the name is related to the crash.
Thinking about it, here it is...
*/
extern "C" int32_t __declspec(dllexport) ignore2(int32_t i)
{
	return 1;
}
