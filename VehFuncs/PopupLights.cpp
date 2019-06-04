#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"

void ProcessPopup(CVehicle *vehicle, ExtendedData *xdata)
{
	float speed = 0.0;
	float ax = 30;
	float ay = 0;
	float az = 0;
	float x = 0;
	float y = 0;
	float z = 0;

	for (int i = 0; i < 2; i++)
	{
		if (xdata->popupFrame[i] != nullptr)
		{
			RwFrame *frame = xdata->popupFrame[i];
			if (frame->object.parent)
			{
				//f_popr=ax45z10s2
				const string name = GetFrameNodeName(frame);


				// Get values (we always need it)
				if (name[6] == '=')
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
							speed = stof(&name[(j + 1)]) * (CTimer::ms_fTimeStep * 1.66666f) * 0.005f;
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

				if (speed == 0.0f)
				{ // default speed value
					speed = 4 * (CTimer::ms_fTimeStep * 1.66666f) * 0.005f;
				}

				// Process if valid
				bool lightsOn;
				if (vehicle->m_nFlags.bLightsOn || vehicle->m_nOverrideLights == 2)
				{
					if (xdata->popupProgress[i] == 1.0f) continue;
					lightsOn = true;
				}
				else
				{
					if (xdata->popupProgress[i] == 0.0f) continue;
					lightsOn = false;
				}

				CAutomobile *automobile = (CAutomobile*)vehicle;
				if (i == LIGHT_FRONT_RIGHT) if (automobile->m_damageManager.GetLightStatus(LIGHT_FRONT_RIGHT)) continue;
				if (i == LIGHT_FRONT_LEFT) if (automobile->m_damageManager.GetLightStatus(LIGHT_FRONT_LEFT)) continue;


				// Up or down
				if (lightsOn) 
					xdata->popupProgress[i] += speed;
				else
					xdata->popupProgress[i] -= speed;
				

				// Update progress and move it
				RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);
				float progress = xdata->popupProgress[i];
				//lg << "popup: " << xdata->popupProgress[i] << " ax " << ax << " ay " << ay << " az " << az << " x " << x << " y " << y << " z " << z << " s " << speed << " \n";
				if (progress >= 1.0f)
				{
					xdata->popupProgress[i] = 1.0f;
					RwV3d vec = { x, y, z };
					RwFrameTranslate(frame, &vec, rwCOMBINEPRECONCAT);
					RwFrameRotate(frame, (RwV3d *)0x008D2E00, ax, rwCOMBINEPRECONCAT);
					RwFrameRotate(frame, (RwV3d *)0x008D2E0C, ay, rwCOMBINEPRECONCAT);
					RwFrameRotate(frame, (RwV3d *)0x008D2E18, az, rwCOMBINEPRECONCAT);
				}
				else
				{
					if (progress <= 0.0f)
					{
						xdata->popupProgress[i] = 0.0f;
						RwV3d vec = { 0, 0, 0 };
						RwFrameTranslate(frame, &vec, rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E00, 0, rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E0C, 0, rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E18, 0, rwCOMBINEPRECONCAT);
					}
					else
					{
						RwV3d vec = { (x * progress), (y * progress), (z * progress) };
						RwFrameTranslate(frame, &vec, rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E00, (0.0f + progress * ax), rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E0C, (0.0f + progress * ay), rwCOMBINEPRECONCAT);
						RwFrameRotate(frame, (RwV3d *)0x008D2E18, (0.0f + progress * az), rwCOMBINEPRECONCAT);
					}
				}
				RwFrameUpdateObjects(frame);
			}
			else
			{
				xdata->popupFrame[i] = nullptr;
				xdata->popupProgress[i] = 0.0f;
			}
		}
	}
}
