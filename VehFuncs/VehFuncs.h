#pragma once
#include "VehFuncsCommon.h"

void ApplyGSX();
RpAtomic *__cdecl CustomMoveObjectsCB(RpAtomic *atomic, RwFrame *frame);
RwFrame *__cdecl CustomCollapseFramesCB(RwFrame *frame, void *data);
RwFrame * CloneNode(RwFrame *frame, RpClump * clump, RwFrame *parent);
void SetWheel(RwFrame * frame[6], CVehicle * vehicle);

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

