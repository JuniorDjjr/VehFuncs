#include "VehFuncsCommon.h"
#include "NodeName.h"

extern float iniDefaultSteerAngle;

void ProcessSteer(CVehicle *vehicle, list<RwFrame*> &frames)
{
	// Taken by reference: this used to copy the whole list every frame, for
	// every vehicle on screen.
	for (list<RwFrame*>::iterator it = frames.begin(); it != frames.end(); )
	{
		RwFrame *frame = *it;
		const bool keep = frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle;
		if (keep) ++it; else it = frames.erase(it);

		if (keep)
		{
			const string name = GetFrameNodeName(frame);

			float angle = (vehicle->m_fSteerAngle * (-1.666666f));

			float maxAngle = iniDefaultSteerAngle;

			if (name[0] == 'f') {
				//f_steer180
				if (isdigit(name[7])) {
					maxAngle = (float)stoi(&name[7]);
				}
				angle *= maxAngle;
			}
			else { //movsteer or steering
				//movsteer_0.5
				maxAngle = 1.0f;
				if (name[8] == '_') {
					if (isdigit(name[9])) {
						maxAngle = stof(&name[9]);
					}
				}
				angle *= 90.0f;
				angle *= maxAngle;
			}
			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);
			RwFrameRotate(frame, (RwV3d*)0x008D2E0C, angle, rwCOMBINEPRECONCAT);
			RwFrameUpdateObjects(frame);
		}
	}
}
