#pragma once
#include "plugin.h"

void FindVehicleCharacteristicsFromNode(RwFrame * frame, CVehicle * vehicle, bool ReSearch);
void SetCharacteristics(CVehicle * vehicle, bool bReSearch);
void SetCharacteristicsInRender(CVehicle * vehicle, bool bReSearch);
void SetCharacteristicsForIndieHandling(CVehicle * vehicle, bool bReSearch);
void ProcessBodyTilt(CVehicle * vehicle);