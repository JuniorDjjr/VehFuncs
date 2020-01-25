#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"

void ProcessRotatePart(CVehicle *vehicle, list<RwFrame*> frames, bool isGear)
{
	for (RwFrame *frame : frames)
	{
		if (frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle)
		{
			const string name = GetFrameNodeName(frame);

			float speedMult = 0.0f + CTimer::ms_fTimeStep * 22.0f;

			size_t found;
			found = name.find("_mu=");
			if (found != string::npos)
			{
				float mult = stof(&name[found + 4]);
				speedMult *= mult;
			}

			if (isGear == true) 
			{
				ExtendedData &xdata = xData.Get(vehicle);
				speedMult += CTimer::ms_fTimeStep * abs(xdata.smoothGasPedal * 13.0f);
			}

			RwV3d *axis = (RwV3d * )0x008D2E0C; //y

			// Find axis
			found = name.find("_x");
			if (found != string::npos)
			{
				axis = (RwV3d *)0x008D2E00;
			}

			//found = name.find("_y");
			//if (found != string::npos)
			//{
			//	axis = (RwV3d *)0x008D2E0C;
			//}

			found = name.find("_z");
			if (found != string::npos)
			{
				axis = (RwV3d *)0x008D2E18;
			}

			RwFrameRotate(frame, axis, speedMult, rwCOMBINEPRECONCAT);

			RwFrameUpdateObjects(frame);
		}
		else
		{
			ExtendedData &xdata = xData.Get(vehicle);
			if (isGear) xdata.gearFrame.remove(frame);
			else xdata.fanFrame.remove(frame);
		}
	}
}
