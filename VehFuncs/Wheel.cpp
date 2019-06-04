#include "VehFuncsCommon.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CVisibilityPlugins.h"

void SetWheel(RwFrame * frame[6], CVehicle * vehicle)
{
	lg << "Wheel: Processing wheel \n";
	for (int j = 0; j < 6; j++)
	{
		if (frame[j])
		{
			if (frame[j]->object.parent)
			{
				const string name = GetFrameNodeName(frame[j]);

				for (int i = 8; i <= 13; i++)
				{
					if (name[i] == '\0') break;
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
								lg << "Wheel: Copying wheel: " << wheelId << " \n";
								CloneNode(frame[j]->child, vehicle->m_pRwClump, wheelFrame);
								CVisibilityPlugins::SetFrameHierarchyId(frame[j]->child, wheelId);
							}
							else
							{
								lg << "Wheel: ERROR: Unable to find wheel node for: " << wheelId << " \n";
							}
						}
					}
				}
				DestroyNodeHierarchyRecursive(frame[j]->child);
				RwFrameDestroy(frame[j]);
				continue;
			}
		}
		ExtendedData &xdata = remInfo.Get(vehicle);
		xdata.wheelFrame[j] = nullptr;
	}
}
