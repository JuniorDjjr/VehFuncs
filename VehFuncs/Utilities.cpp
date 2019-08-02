#include "VehFuncsCommon.h"
#include "CBaseModelInfo.h"
#include "CVisibilityPlugins.h"
#include "CModelInfo.h"
#include "CBike.h"
#include "NodeName.h"
#include "MatrixBackup.h"

float GetVehicleSpeedRealistic(CVehicle * vehicle)
{
	float wheelSpeed = 0.0;
	CVehicleModelInfo * vehicleModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(vehicle->m_nModelIndex);
	if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX)
	{
		CBike * bike = (CBike *)vehicle;
		wheelSpeed = ((bike->m_fWheelSpeed[0] * vehicleModelInfo->m_fWheelSizeFront) +
			(bike->m_fWheelSpeed[1] * vehicleModelInfo->m_fWheelSizeRear)) / 2.0f;
	}
	else if (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_MTRUCK || vehicle->m_nVehicleSubClass == VEHICLE_QUAD)
	{
		CAutomobile * automobile = (CAutomobile *)vehicle;
		wheelSpeed = ((automobile->m_fWheelSpeed[0] + automobile->m_fWheelSpeed[1] * vehicleModelInfo->m_fWheelSizeFront) +
			(automobile->m_fWheelSpeed[2] + automobile->m_fWheelSpeed[3] * vehicleModelInfo->m_fWheelSizeRear)) / 4.0f;
	}
	else
	{
		return (vehicle->m_vecMoveSpeed.Magnitude() * 50.0f) * 3.6f;
	}
	wheelSpeed /= 2.45f; // tweak based on distance (manually testing)
	wheelSpeed *= -186.0f; // tweak based on km/h

	return wheelSpeed;
}

// -- Exported functions

extern "C" int32_t __declspec(dllexport) Ext_GetVehicleSpeedRealistic(CVehicle * vehicle)
{
	float fSpeed = GetVehicleSpeedRealistic(vehicle);
	return *reinterpret_cast<int32_t*>(addressof(fSpeed));
}


extern "C" int32_t __declspec(dllexport) Ext_GetMDPMnpcCustomChance(CVehicle * vehicle)
{
	ExtendedData &xdata = remInfo.Get(vehicle);
	
	return xdata.mdpmCustomChances;
}

extern "C" int32_t __declspec(dllexport) Ext_GetMDPMnpcCustomMinVol(CVehicle * vehicle)
{
	ExtendedData &xdata = remInfo.Get(vehicle);

	return *reinterpret_cast<int32_t*>(addressof(xdata.mdpmCustomMinVol));
}

extern "C" int32_t __declspec(dllexport) Ext_GetMDPMnpcCustomMaxVol(CVehicle * vehicle) 
{
	ExtendedData &xdata = remInfo.Get(vehicle);

	return *reinterpret_cast<int32_t*>(addressof(xdata.mdpmCustomMaxVol));
}

extern "C" void __declspec(dllexport) Ext_ToggleFrameHide(RwFrame * frame, bool neverRender)
{
	FRAME_EXTENSION(frame)->flags.bNeverRender = neverRender;
	return;
}

/*
The function name appears for crash dumps as modloader.log, and people mistakenly think the name is related to the crash.
Thinking about it, here it is...
*/
extern "C" int32_t __declspec(dllexport) ignore(int32_t i)
{
	return 1;
}

void CloneNodeAlt(RwFrame *frame, RwFrame *parent, bool isRoot)
{
	RwFrame * newFrame;

	newFrame = RwFrameCreate();

	//CVisibilityPlugins::SetFrameHierarchyId(newFrame, 101);

	memcpy(&newFrame->modelling, &frame->modelling, sizeof(RwMatrix));

	const string frameName = GetFrameNodeName(frame);
	SetFrameNodeName(newFrame, &frameName[0]);

	RwFrameForAllObjects(frame, CopyObjectsCB, parent);

	RwFrameAddChild(parent, newFrame);

	/////////////////////////////////////////
	if (RwFrame * nextFrame = frame->child) CloneNodeAlt(nextFrame, newFrame, false);
	if (!isRoot) if (RwFrame * nextFrame = frame->next)  CloneNodeAlt(nextFrame, parent, false);
	return;
}

void CloneNode(RwFrame *frame, RpClump * clump, RwFrame *parent, bool isRoot, bool isVarWheel)
{
	RwFrame * newFrame;

	if (isVarWheel) FRAME_EXTENSION(frame)->flags.bIsVarWheel = true;

	if (isRoot)
	{
		*(uint32_t*)0xC1CB58 = (uint32_t)clump;
		RwFrameForAllObjects(frame, CopyObjectsCB, parent);
		if (RwFrame * nextFrame = frame->child) CloneNode(nextFrame, clump, parent, false, isVarWheel);
	}
	else
	{
		newFrame = RwFrameCreate();

		//CVisibilityPlugins::SetFrameHierarchyId(newFrame, 101);

		memcpy(&newFrame->modelling, &frame->modelling, sizeof(RwMatrix));
		RwMatrixUpdate(&newFrame->modelling);

		const string frameName = GetFrameNodeName(frame);
		SetFrameNodeName(newFrame, &frameName[0]);

		RpAtomic * atomic = (RpAtomic*)GetFirstObject(frame);

		if (atomic)
		{
			RpAtomic * newAtomic = RpAtomicClone(atomic);
			RpAtomicSetFrame(newAtomic, newFrame);
			RpClumpAddAtomic(clump, newAtomic);
			//CVisibilityPlugins::SetAtomicRenderCallback(newAtomic, (RpAtomic *(*)(RpAtomic *))0x007323C0);
		}

		RwFrameAddChild(parent, newFrame);

		if (RwFrame * nextFrame = frame->child) CloneNode(nextFrame, clump, newFrame, false, isVarWheel);
		if (RwFrame * nextFrame = frame->next)  CloneNode(nextFrame, clump, parent, false, isVarWheel);
	}
	return;
}

void DestroyNodeHierarchyRecursive(RwFrame * frame)
{
	RpAtomic * atomic = (RpAtomic *)GetFirstObject(frame);
	if (atomic != nullptr) 
	{
		RpClump * clump = atomic->clump;
		RpClumpRemoveAtomic(clump, atomic);
		RpAtomicDestroy(atomic);
	}
	
	RwFrameDestroy(frame);

	/////////////////////////////////////////
	if (RwFrame * newFrame = frame->child) DestroyNodeHierarchyRecursive(newFrame);
	if (RwFrame * newFrame = frame->next)  DestroyNodeHierarchyRecursive(newFrame);

	return;
}
