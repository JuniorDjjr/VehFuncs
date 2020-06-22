#pragma once
#include "VehFuncsCommon.h"

void ApplyGSX();
RpAtomic *__cdecl CustomMoveObjectsCB(RpAtomic *atomic, RwFrame *frame);
RwFrame *__cdecl CustomCollapseFramesCB(RwFrame *frame, void *data);
void SetWheel(RwFrame * frame[6], CVehicle * vehicle);
RwFrame *__cdecl CustomRwFrameForAllChildren_RemoveUpgrade(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data);
RwFrame *__cdecl CustomRwFrameForAllChildren_RemoveUpgrade_Recurse(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data);
RwFrame *__cdecl CustomRwFrameForAllChildren_AddUpgrade(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data);
RwFrame *__cdecl CustomRwFrameForAllChildren_AddUpgrade_Recurse(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data);
RwFrame *__cdecl CustomRwFrameForAllObjects_Upgrades(RwFrame *frame, RpAtomicCallBack callback, void *data);

ThiscallEvent <AddressList<0x6D6494, H_CALL>, PRIORITY_AFTER, ArgPickN<CVehicle*, 0>, void(CVehicle*)> vehiclePreRenderEvent;

void LogLastVehicleRendered();
void LogCrashText(string str);
void LogVehicleModelWithText(string str1, int vehicleModel, string str2);

extern int lastRenderedVehicleModel;
unsigned int FramePluginOffset;

extern VehicleExtendedData<ExtendedData> &getExtData()
{
	static VehicleExtendedData<ExtendedData> xData;
	return xData;
}

extern fstream &logfile()
{
	static fstream fs;
	return fs;
}

extern list<string> &getClassList()
{
	static list<string> classList;
	return classList;
}

