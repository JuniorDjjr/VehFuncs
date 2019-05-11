#pragma once
#include "plugin.h"

void StoreHandlingData();
extern "C" int __declspec(dllexport) IsIndieHandling(CVehicle *veh, tHandlingData **pHandling_to);
bool ExtraInfoBitReSearch(CVehicle * vehicle, tHandlingData * handling);
