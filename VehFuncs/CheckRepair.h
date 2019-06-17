#pragma once
struct repairCallbackStructure
{
	CVehicle *veh;
};

typedef void(__cdecl externalCbFun_t)(const repairCallbackStructure*);
typedef std::vector<std::function<externalCbFun_t>> externalRepairCallbacks_t;

void ApplyCheckRepair();
extern externalRepairCallbacks_t &getRepairCallbacks();
extern externalRepairCallbacks_t &getTempCallbacks();