#include "VehFuncsCommon.h"
#include "NodeName.h"

void ProcessSteer(CVehicle *vehicle, RwFrame *frame)
{
	const string name = GetFrameNodeName(frame);

	float angle = (vehicle->m_fSteerAngle * (-1.666666f));

	float maxAngle = 100.0f;

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
