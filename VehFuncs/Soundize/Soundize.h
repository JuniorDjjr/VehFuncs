#pragma once
#include "plugin.h"

extern "C" void __declspec(dllexport) Ext_SetVehicleBackfireMode(CVehicle * vehicle, int mode);
extern "C" void __declspec(dllexport) Ext_SetVehicleSubDefinitionName(CVehicle * vehicle, char* pSubDefinitionName);
extern "C" int __declspec(dllexport) Ext_GetVehicleDoingBackfire(CVehicle * vehicle);