#include "plugin.h"
#include "VehFuncsCommon.h"
#include "CheckRepair.h"
#include "injectorAddons/injectorAddons.hpp"
#include <array>

extern fstream lg;

extern externalRepairCallbacks_t &getRepairCallbacks()
{
	static externalRepairCallbacks_t externalCb;
	return externalCb;
}

extern externalRepairCallbacks_t &getTempCallbacks()
{
	static externalRepairCallbacks_t externalCb;
	return externalCb;
}

void callexternalCallbacks(repairCallbackStructure &e)
{
	for (auto &cb : getRepairCallbacks())
	{
		cb(&e);
	}

	for (auto &cb : getTempCallbacks())
	{
		cb(&e);
	}
}

void onload(int id)
{
	getTempCallbacks().clear();
}


void addRepairCallback(externalCbFun_t fun)
{
	getRepairCallbacks().push_back(fun);
}

void addRepairTempCallback(externalCbFun_t fun)
{
	getTempCallbacks().push_back(fun);
}


void repairCallback(const repairCallbackStructure *test)
{
	if (test->veh)
	{
		ExtendedData &xdata = remInfo.Get(test->veh);

		xdata.flags.bDamageUpdated = true;
	}
	return;
}


void ApplyCheckRepair (){
	addRepairCallback(repairCallback);

	injectorAddons::MakeInlineAutoCallOriginal<0x006A04E3>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.edi;
		callexternalCallbacks(cbdata);
	});

	injectorAddons::MakeInlineAutoCallOriginal<0x006A3466>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.edi;
		callexternalCallbacks(cbdata);
	});

	//injectorAddons::MakeInlineAutoCallOriginal<0x006B0AD8>([](reg_pack &regs) {
	//	repairCallbackStructure cbdata;
	//	cbdata.veh = (CVehicle *)regs.esi;
	//	callexternalCallbacks(cbdata);
	//});

	injectorAddons::MakeInlineAutoCallOriginal<0x006C4539>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.esi;
		callexternalCallbacks(cbdata);
	});

	injectorAddons::MakeInlineAutoCallOriginal<0x006CABBC>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.edi;
		callexternalCallbacks(cbdata);
	});

	injectorAddons::MakeInlineAutoCallOriginal<0x006CE2BC>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.edi;
		callexternalCallbacks(cbdata);
	});

	// CVehicle::SetComponentVisibility
	injectorAddons::MakeInlineAutoCallOriginal<0x006D2721>([](reg_pack &regs) {
		repairCallbackStructure cbdata;
		cbdata.veh = (CVehicle *)regs.ecx;
		callexternalCallbacks(cbdata);
	});

}


