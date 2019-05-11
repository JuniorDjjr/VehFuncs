#pragma once
#include "plugin.h"

extern "C" int32_t __declspec(dllexport) Ext_GetVehicleSpeedRealistic(CVehicle * vehicle);

float GetVehicleSpeedRealistic(CVehicle * vehicle);
void DestroyNodeHierarchyRecursive(RwFrame * frame);
void CloneNode(RwFrame *frame, RpClump * clump, RwFrame *parent);