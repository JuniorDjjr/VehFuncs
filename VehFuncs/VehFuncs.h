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

unsigned int FramePluginOffset;

extern VehicleExtendedData<ExtendedData> &getremInfo()
{
	static VehicleExtendedData<ExtendedData> remInfo;
	return remInfo;
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

