#include "VehFuncsCommon.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CVisibilityPlugins.h"

extern bool createCloneNoRender;

void SetWheel(RwFrame * frame[6], CVehicle * vehicle)
{
	if (useLog) lg << "Wheel: Processing wheel \n";
	bool hasTuningWheel = (int)vehicle->m_anUpgrades[0] >= 0;
	//lg << "Wheel: upgrade " << (int)hasTuningWheel << std::endl;
	for (int j = 0; j < 6; j++)
	{
		if (frame[j])
		{
			if (frame[j]->object.parent && FRAME_EXTENSION(frame[j])->owner == vehicle)
			{
				const string name = GetFrameNodeName(frame[j]);

				for (int i = 8; i <= 13; i++)
				{
					if (name[i] == '\0' || name[i] == ':' || name[i] == '?') break;
					if (name[i] == '1')
					{
						int wheelId;
						int subClass = vehicle->m_nVehicleSubClass;

						switch (i)
						{
						case 8:
							if (subClass == 9 || subClass == 10) wheelId = 4;
							else wheelId = CAR_WHEEL_RF;
							break;
						case 9:
							if (subClass == 9 || subClass == 10) wheelId = 5;
							else wheelId = CAR_WHEEL_RB;
							break;
						case 10:
							wheelId = CAR_WHEEL_LB;
							break;
						case 11:
							wheelId = CAR_WHEEL_LF;
							break;
						case 12:
							wheelId = CAR_WHEEL_RM;
							break;
						case 13:
							wheelId = CAR_WHEEL_LM;
							break;
						default:
							wheelId = 0;
							break;
						}
						if (wheelId)
						{
							RwFrame * wheelFrame;
							if (subClass == 9 || subClass == 10) {
								wheelFrame = CClumpModelInfo::GetFrameFromId(vehicle->m_pRwClump, wheelId);
							}
							else
							{
								wheelFrame = reinterpret_cast<CAutomobile*>(vehicle)->m_aCarNodes[wheelId]; // this doesn't work for bikes, idkw
							}

							if (wheelFrame) {
								if (useLog) lg << "Wheel: Copying wheel: " << wheelId << " \n";
								createCloneNoRender = hasTuningWheel;
								CloneNode(frame[j]->child, vehicle->m_pRwClump, wheelFrame, false, true);
								createCloneNoRender = false;
								CVisibilityPlugins::SetFrameHierarchyId(frame[j]->child, wheelId);
							}
							else
							{
								if (useLog) lg << "Wheel: ERROR: Unable to find wheel node for: " << wheelId << " \n";
							}
						}
					}
				}
				DestroyNodeHierarchyRecursive(frame[j]->child);
				RwFrameDestroy(frame[j]);
				continue;
			}
		}
		ExtendedData &xdata = xData.Get(vehicle);
		xdata.wheelFrame[j] = nullptr;
	}
}
