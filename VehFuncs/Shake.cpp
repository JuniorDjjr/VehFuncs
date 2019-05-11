#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "PerlinNoise\SimplexNoise.h"
#include "MatrixBackup.h"

void ProcessShake(CVehicle *vehicle, list<RwFrame*> frames)
{
	ExtendedData &xdata = remInfo.Get(vehicle);

	// Process dot
	xdata.dotLife += CTimer::ms_fTimeStep * (0.2f * ((xdata.smoothGasPedal * 2.0f) + 1.0f));
	if (xdata.dotLife >= 100.0f) xdata.dotLife = 1.0f;

	for (list<RwFrame*>::iterator it = frames.begin(); it != frames.end(); ++it)
	{
		RwFrame * frame = *it;
		if (frame->object.parent)
		{
			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);

			if (vehicle->m_nFlags.bEngineOn && !vehicle->m_nFlags.bEngineBroken)
			{
				// Get noise
				float noise = SimplexNoise::noise(xdata.dotLife);

				const string name = GetFrameNodeName(frame);
				size_t found;

				// Mult noise
				found = name.find("_mu=");
				if (found != string::npos)
				{
					float mult = stof(&name[found + 4]);
					noise *= mult;
				}

				// Div noise by gas pedal 
				//noise /= ((xdata.smoothGasPedal * 1.0f) + 1.0f);

				// Convert noise to shake (angle)
				float angle = 0.0f;
				angle += noise * 0.7f;

				// Apply tilt
				found = name.find("_tl=");
				if (found != string::npos)
				{
					float angMult = stof(&name[found + 6]);
					angle += xdata.smoothGasPedal * angMult;
				}

				// Find axis
				RwV3d * axis;
				found = name.find("_x");
				if (found != string::npos)
				{
					axis = (RwV3d*)0x008D2E00;
				}
				else
				{
					found = name.find("_z");
					if (found != string::npos)
					{
						axis = (RwV3d*)0x008D2E18;
					}
					else axis = (RwV3d*)0x008D2E0C;
				}

				// Rotate
				RwFrameRotate(frame, axis, angle, rwCOMBINEPRECONCAT);
			}
			RwFrameUpdateObjects(frame);
		}
		else
		{
			ExtendedData &xdata = remInfo.Get(vehicle);
			xdata.shakeFrame.remove(*it);
		}
	}
}
