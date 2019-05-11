#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "MatrixBackup.h"

void ProcessPedal(CVehicle *vehicle, list<RwFrame*> frames, int mode)
{
	for (list<RwFrame*>::iterator it = frames.begin(); it != frames.end(); ++it)
	{
		RwFrame * frame = *it;
		if (frame->object.parent)
		{
			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);

			if (vehicle->m_nFlags.bEngineOn && !vehicle->m_nFlags.bEngineBroken)
			{
				ExtendedData &xdata = remInfo.Get(vehicle);

				const string name = GetFrameNodeName(frame);
				size_t found;

				float angle = 0.0;
				float pedal;

				if (mode == 1)
				{
					pedal = xdata.smoothGasPedal;
				}
				else 
				{
					pedal = xdata.smoothBrakePedal;
				} 

				// Find axis
				found = name.find("_ax=");
				if (found != string::npos)
				{
					float angleX = stof(&name[found + 4]);
					angle = 0.0f + pedal * angleX;
					// Rotate
					RwFrameRotate(frame, (RwV3d*)0x008D2E00, angle, rwCOMBINEPRECONCAT);
				}

				found = name.find("_ay=");
				if (found != string::npos)
				{
					float angleZ = stof(&name[found + 4]);
					angle = 0.0f + pedal * angleZ;
					// Rotate
					RwFrameRotate(frame, (RwV3d*)0x008D2E0C, angle, rwCOMBINEPRECONCAT);
				}

				found = name.find("_az=");
				if (found != string::npos)
				{
					float angleZ = stof(&name[found + 4]);
					angle = 0.0f + pedal * angleZ;
					// Rotate
					RwFrameRotate(frame, (RwV3d*)0x008D2E18, angle, rwCOMBINEPRECONCAT);
				}
			}
			RwFrameUpdateObjects(frame);
		}
		else
		{
			ExtendedData &xdata = remInfo.Get(vehicle);
			if (mode == 1) xdata.gaspedalFrame.remove(*it); else xdata.brakepedalFrame.remove(*it);
		}
	}
}

