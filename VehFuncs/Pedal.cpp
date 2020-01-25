#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "MatrixBackup.h"

void ProcessPedal(CVehicle *vehicle, list<RwFrame*> frames, int mode)
{
	for (RwFrame *frame : frames)
	{
		if (frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle)
		{
			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);

			if (vehicle->m_nVehicleFlags.bEngineOn && vehicle->m_fHealth > 0 && !vehicle->m_nVehicleFlags.bEngineBroken && !vehicle->m_nVehicleFlags.bIsDrowning)
			{
				ExtendedData &xdata = xData.Get(vehicle);

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
			ExtendedData &xdata = xData.Get(vehicle);
			if (mode == 1) xdata.gaspedalFrame.remove(frame); else xdata.brakepedalFrame.remove(frame);
		}
	}
}

