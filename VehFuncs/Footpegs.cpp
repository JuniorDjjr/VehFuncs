#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "MatrixBackup.h"

void ProcessFootpegs(CVehicle *vehicle, list<RwFrame*> frames, int mode)
{
	for (list<RwFrame*>::iterator it = frames.begin(); it != frames.end(); ++it)
	{
		RwFrame * frame = *it;
		if (frame->object.parent)
		{
			bool open = false;

			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);
			RwMatrixUpdate(&frame->modelling);

			if (mode == 1) 
			{
				if (vehicle->m_pDriver) open = true; else open = false;
			}
			else 
			{
				if (vehicle->m_apPassengers[0]) open = true; else open = false;
			}

			if (open) 
			{
				ExtendedData &xdata = remInfo.Get(vehicle);

				const string name = GetFrameNodeName(frame);
				size_t found;

				float angle = 0.0;

				// Find axis
				found = name.find("_ax=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]);
					RwFrameRotate(frame, (RwV3d*)0x008D2E00, angle, rwCOMBINEPRECONCAT);
				}

				found = name.find("_ay=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]);
					RwFrameRotate(frame, (RwV3d*)0x008D2E0C, angle, rwCOMBINEPRECONCAT);
				}

				found = name.find("_az=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]);
					RwFrameRotate(frame, (RwV3d*)0x008D2E18, angle, rwCOMBINEPRECONCAT);
				}
				RwFrameUpdateObjects(frame);
			}
		}
		else
		{
			ExtendedData &xdata = remInfo.Get(vehicle);
			if (mode == 1) xdata.fpeg1Frame.remove(*it); else xdata.fpeg2Frame.remove(*it);
		}
	}
}

