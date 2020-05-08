#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "MatrixBackup.h"
#include "Anims.h"

const float DEFAULT_SPEED = 0.005f;

void ProcessAnims(CVehicle *vehicle, list<F_an*> items)
{
	for (F_an *an : items)
	{
		// we will not make previous-values-compatibility here
		// TODO: set this during the part store
		float speed = 0.0;
		float ax = 0;
		float ay = 0;
		float az = 0;
		float x = 0;
		float y = 0;
		float z = 0;

		RwFrame *frame = an->frame;
		if (frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle)
		{
			bool open = false;
			int mode = an->mode;
			int submode = an->submode;

			switch (mode)
			{
			case 0:
				//if (useLog) lg << "Anims: Found 'f_an" << mode << "': ping pong \n";
				if (an->progress == 1.0f)
				{
					if (an->opening)
					{
						open = false;
						an->opening = false;
					}
					else
					{
						open = false;
					}
				}
				else
				{
					if (an->progress == 0.0f)
					{
						open = true;
						an->opening = true;
					}
					else
					{
						open = an->opening;
					}
				}
				break;
				break;
			case 1:
				switch (submode)
				{
				case 0:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": engine off \n";
					if (!vehicle->m_nVehicleFlags.bEngineOn)
						open = true;
					else
						open = false;
					break;
				case 1:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": engine off or alarm on \n";
					if (!vehicle->m_nVehicleFlags.bEngineOn)
						open = true;
					else
						open = false;
					break;
				}
				break;
			case 2:
				switch (submode)
				{
				case 0:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": driver \n";
					if (vehicle->m_pDriver)
						open = true;
					else
						open = false;
					break;
				case 1:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 1 \n";
					if (vehicle->m_apPassengers[0])
						open = true;
					else
						open = false;
					break;
				case 2:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 2 \n";
					if (vehicle->m_apPassengers[1])
						open = true;
					else
						open = false;
					break;
				case 3:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 3 \n";
					if (vehicle->m_apPassengers[2])
						open = true;
					else
						open = false;
					break;
				}
				break;
			case 3:
				switch (submode)
				{
				case 0:
				case 1:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed \n";
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed and f_spoiler \n";
					if (((vehicle->m_vecMoveSpeed.Magnitude() * 50.0f) * 3.6f) > 100.0f)
						open = true;
					else
						open = false;
					break;
				}
				break;
			case 4:
				switch (submode)
				{
				case 0:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": brake \n";
					if (vehicle->m_fBreakPedal > 0.5f)
						open = true;
					else
						open = false;
					break;
				case 1:
				case 2:
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed brake \n";
					//if (useLog) lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed brake and f_spoiler \n";
					if (vehicle->m_fBreakPedal > 0.5f && ((vehicle->m_vecMoveSpeed.Magnitude() * 50.0f) * 3.6f) > 100.0f)
						open = true;
					else
						open = false;
					break;
				}
				break;
			}


			if (!mode == 0)
			{
				if (open)
				{
					if (an->progress == 1.0f) continue;
				}
				else
				{
					if (an->progress == 0.0f) continue;
				}
			}

			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);

			ExtendedData &xdata = xData.Get(vehicle);


			//f_an1a=ax45z10s2
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
				for (unsigned int j = 6; j < name.length(); j++)
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

			// Default speed value
			if (speed == 0.0f)
			{
				speed = 4 * (CTimer::ms_fTimeStep * 1.66666f) * DEFAULT_SPEED;
			}

			// Update progress and move it
			if (open)
			{
				an->progress += speed;
				if (an->progress > 1.0f) an->progress = 1.0f;
			}
			else
			{
				an->progress -= speed;
				if (an->progress < 0.0f) an->progress = 0.0f;
			}

			float progress = an->progress;
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
			delete an;
			xdata.anims.remove(an);
		}
	}
}

