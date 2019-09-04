#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "MatrixBackup.h"
#include "Footpegs.h"

const float DEFAULT_SPEED = 0.01f;

void ProcessFootpegs(CVehicle *vehicle, list<F_footpegs*> items, int mode)
{
	float speed = 0.0;
	float ax = 90;
	float ay = 0;
	float az = 0;
	float x = 0;
	float y = 0;
	float z = 0;

	for (list<F_footpegs*>::iterator it = items.begin(); it != items.end(); ++it)
	{
		F_footpegs *footpegs = *it;
		RwFrame *frame = footpegs->frame;
		if (frame->object.parent)
		{
			bool open = false;

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
				if (footpegs->progress == 1.0f) continue;
			}
			else
			{
				if (footpegs->progress == 0.0f) continue;
			}


			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);

			ExtendedData &xdata = xData.Get(vehicle);


			//f_fpeg1=ax45z10s2
			const string name = GetFrameNodeName(frame);
			size_t found;


			// Get values (we always need it)
			if (name[7] == '=')
			{
				speed = 0;
				ax = 0;
				ay = 0;
				az = 0;
				x = 0;
				y = 0;
				z = 0;
				for (unsigned int j = 7; j < name.length(); j++)
				{
					switch (name[j])
					{

					case 'a':
						switch (name[(j + 1)])
						{
						case 'x':
							ax = stof(&name[(j + 2)]);
							break;
						case 'y':
							ay = stof(&name[(j + 2)]);
							break;
						case 'z':
							az = stof(&name[(j + 2)]);
							break;
						}
						break;

					case 's':
						speed = stof(&name[(j + 1)]) * (CTimer::ms_fTimeStep * 1.66666f) * DEFAULT_SPEED;
						break;

					case 'x':
						if (name[(j - 1)] == 'a') break;
						x = stof(&name[(j + 1)]) * 0.01f;
						break;

					case 'y':
						if (name[(j - 1)] == 'a') break;
						y = stof(&name[(j + 1)]) * 0.01f;
						break;

					case 'z':
						if (name[(j - 1)] == 'a') break;
						z = stof(&name[(j + 1)]) * 0.01f;
						break;
					}

					if (name[j] == '_') break;
				}
			}
			else 
			{
				// retrocompatibility
				if (open)
				{
					if (footpegs->progress == 1.0f) continue;
					footpegs->progress += 0.2f * (CTimer::ms_fTimeStep / 1.66666f);
					if (footpegs->progress > 1.0f) footpegs->progress = 1.0f;
				}
				else
				{
					if (footpegs->progress == 0.0f) continue;
					footpegs->progress -= 0.2f * (CTimer::ms_fTimeStep / 1.66666f);
					if (footpegs->progress < 0.0f) footpegs->progress = 0.0f;
				}
				float angle;
				found = name.find("_ax=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]) * footpegs->progress;
					RwFrameRotate(frame, (RwV3d*)0x008D2E00, angle, rwCOMBINEPRECONCAT);
				}
				found = name.find("_ay=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]) * footpegs->progress;
					RwFrameRotate(frame, (RwV3d*)0x008D2E0C, angle, rwCOMBINEPRECONCAT);
				}
				found = name.find("_az=");
				if (found != string::npos)
				{
					angle = stof(&name[found + 4]) * footpegs->progress;
					RwFrameRotate(frame, (RwV3d*)0x008D2E18, angle, rwCOMBINEPRECONCAT);
				}
				RwFrameUpdateObjects(frame);
				continue;
			}

			// Default speed value
			if (speed == 0.0f)
			{
				speed = 4 * (CTimer::ms_fTimeStep * 1.66666f) * DEFAULT_SPEED;
			}

			// Update progress and move it
			if (open)
			{
				footpegs->progress += speed;
				if (footpegs->progress > 1.0f) footpegs->progress = 1.0f;
			}
			else
			{
				footpegs->progress -= speed;
				if (footpegs->progress < 0.0f) footpegs->progress = 0.0f;
			}

			float progress = footpegs->progress;
			if (progress != 0.0f)
			{
				if (x != 0 || y != 0 || z != 0)
				{
					RwV3d vec = { (x * progress), (y * progress), (z * progress) };
					RwFrameTranslate(frame, &vec, rwCOMBINEPRECONCAT);
				}
				if (ax != 0) RwFrameRotate(frame, (RwV3d *)0x008D2E00, (0.0f + progress * ax), rwCOMBINEPRECONCAT);
				if (ay != 0) RwFrameRotate(frame, (RwV3d *)0x008D2E0C, (0.0f + progress * ay), rwCOMBINEPRECONCAT);
				if (az != 0) RwFrameRotate(frame, (RwV3d *)0x008D2E18, (0.0f + progress * az), rwCOMBINEPRECONCAT);
			}
			RwFrameUpdateObjects(frame);
		}
		else
		{
			ExtendedData &xdata = xData.Get(vehicle);
			F_footpegs *footpegs = *it;
			delete footpegs;
			if (mode == 1) xdata.fpegFront.remove(*it); else xdata.fpegBack.remove(*it);
		}
	}
}

