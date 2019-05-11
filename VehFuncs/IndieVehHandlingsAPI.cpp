#include <game_sa\cHandlingDataMgr.h>
#include "IndieVehHandlingsAPI.h"
#include <bitset>
#include <sstream>

using namespace std;
using namespace injector;

extern fstream lg;

struct handlingBlocKInfo
{
	void *start, *end;
	int size;

	handlingBlocKInfo() : start(nullptr), end(nullptr), size(0) { }
} hblockinfo;

#pragma pack(push, 1)
struct fastmansPatchData
{
	/*
	* Copyright (C) 2014-2017  fastman92 <fastman92@gmail.com>, website: http://fastman92.com
	* Licensed under the MIT License, see LICENSE at top level directory.
	*
	*/

	float field_0;
	float field_4;
	float field_8;
	float field_C;
	float field_10;

	uint32_t cHandlingDataMgr_extended_magicID;
	uint32_t sizeof_cHandlingDataMgr_header;

	unsigned int numberOfVehicleLines;
	unsigned int numberOfBikeLines;
	unsigned int numberOfFlyingLines;
	unsigned int numberOfBoatLines;

	// offsets
	uint32_t m_aStandardHandling_offset;
	uint32_t m_aBikeHandling_offset;
	uint32_t m_aFlyingHandling_offset;
	uint32_t m_aBoatHandling_offset;
	void* m_pVehicleAnimGroups;

	// sizes
	uint32_t m_sizeof_tHandlingData;
	uint32_t m_sizeof_tBikeHandlingData;
	uint32_t m_sizeof_tFlyingHandlingData;
	uint32_t m_sizeof_tBoatHandlingData;
	uint32_t m_sizeof_CAnimGroup;

	// limits
	uint32_t m_iStandardLinesLimit;
	uint32_t m_iBikeLinesLimit;
	uint32_t m_iFlyingLinesLimit;
	uint32_t m_iBoatLinesLimit;
	uint32_t m_iAnimationGroupLinesLimit;
};
#pragma pack(pop)

extern "C" int __declspec(dllexport) IsIndieHandling(CVehicle *veh, tHandlingData **pHandling_to)
{
	*pHandling_to = veh->m_pHandlingData;

	if (hblockinfo.start <= veh->m_pHandlingData && veh->m_pHandlingData < hblockinfo.end)
	{
		return false;
	}
	return true;
}

void StoreHandlingData()
{
	cHandlingDataMgr *handlingManager = ReadMemory<cHandlingDataMgr*>(0x005BFA95 + 1, true);

	if (static_cast<uint32_t>(handlingManager->m_aVehicleHandling[0].m_nVehicleId) == 0xFFFFFFFFu)
	{
		// fastman's patch
		const fastmansPatchData &patchData = *reinterpret_cast<fastmansPatchData*>(handlingManager);

		hblockinfo.size = patchData.m_sizeof_tHandlingData;
		hblockinfo.start = reinterpret_cast<uint8_t*>(handlingManager) + patchData.m_aStandardHandling_offset;
		hblockinfo.end = reinterpret_cast<uint8_t*>(handlingManager) + (patchData.m_sizeof_tHandlingData * patchData.numberOfVehicleLines) + patchData.m_aStandardHandling_offset;

		lg << "Handling: fastman92's limit adjuster pointer: " << hblockinfo.start << " " << hblockinfo.size << " " << hblockinfo.end << " \n";
	}
	else
	{
		if (handlingManager == reinterpret_cast<cHandlingDataMgr*>(0x00C2B9C8))
		{
			// vanilla
			hblockinfo.size = 224;
			hblockinfo.start = reinterpret_cast<uint8_t*>(handlingManager) + 0x14;
			hblockinfo.end = reinterpret_cast<uint8_t*>(handlingManager) + ReadMemory<uint32_t>(0x006F531E, true) + 0x14;

			lg << "Handling: Vanilla handling pointer: " << hblockinfo.start << " " << hblockinfo.size << " " << hblockinfo.end << " \n";
		}
		else
		{
			lg << "Handling: (ERROR) Unknown limit adjuster: " << handlingManager << " \n";
			lg.flush();
			MessageBoxA(0, "Unknown limit adjuster", "VehFuncs", 0);
			throw runtime_error("VehFuncs Error: Unknown limit adjuster");
		}
	}
}

bool ExtraInfoBitReSearch(CVehicle * vehicle, tHandlingData * handling)
{
	int ihandling = (int)handling + 0xF7; // Extra Infos start (0xE5+0x12)
	bitset<32> flags = ReadMemory<uint32_t>(ihandling, false);

	if (flags.test(0))
	{
		if (!flags.test(1))
		{
			lg << "ReSearch: Reapplying vehicle \n";
			flags.set(1);
			WriteMemory<uint32_t>(ihandling, flags.to_ulong(), false);
			return true;
		}
	}
	else if (flags.test(1))
	{
		flags.reset(1);
		WriteMemory<uint32_t>(ihandling, flags.to_ulong(), false);
	}
	return false;
}