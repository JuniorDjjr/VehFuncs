#pragma once
#include "plugin.h"

typedef void(__cdecl* Ext_SetVehicleSubDefinitionName)(CVehicle* vehicle, char* pSubDefinitionName);
extern Ext_SetVehicleSubDefinitionName sz_Ext_SetVehicleSubDefinitionName;