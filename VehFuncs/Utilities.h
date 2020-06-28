#pragma once
#include "plugin.h"

extern "C" int32_t __declspec(dllexport) Ext_GetVehicleSpeedRealistic(CVehicle * vehicle);

float GetVehicleSpeedRealistic(CVehicle * vehicle);
void DestroyNodeHierarchyRecursive(RwFrame * frame);
void CloneNode(RwFrame *frame, RpClump * clump, RwFrame *parent, bool isRoot, bool isVarWheel);
void CloneNodeAlt(RwFrame *frame, RwFrame *parent, bool isRoot);
int GetDefaultLodForInteriorMinor(CVehicle *vehicle);
bool ChangePedModel(CPed *ped, int model, CVehicle *vehicle, int passengerId);
int RandomRange(int min, int max);