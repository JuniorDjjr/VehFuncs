#include "Matrixbackup.h"
#include "VehFuncsCommon.h"
extern std::fstream lg;

void RestoreMatrixBackup(RwMatrix* dest, MatrixBackup* backup)
{
	if (backup != nullptr) 
	{
		dest->at.x = backup->at.x;
		dest->at.y = backup->at.y;
		dest->at.z = backup->at.z;

		dest->pos.x = backup->pos.x;
		dest->pos.y = backup->pos.y;
		dest->pos.z = backup->pos.z;

		dest->right.x = backup->right.x;
		dest->right.y = backup->right.y;
		dest->right.z = backup->right.z;

		dest->up.x = backup->up.x;
		dest->up.y = backup->up.y;
		dest->up.z = backup->up.z;

		RwMatrixUpdate(dest);
	}
	else
	{
		if (useLog) lg << "Error: Unable to restore matrix backup A\n";
		if (useLog) lg.flush();
	}
	return;
}

bool CreateMatrixBackup(RwFrame* frame) 
{
	MatrixBackup *backup = new MatrixBackup();
	if (backup)
	{
		RwMatrix* origin = &frame->modelling;
		backup->at.x = origin->at.x;
		backup->at.y = origin->at.y;
		backup->at.z = origin->at.z;

		backup->pos.x = origin->pos.x;
		backup->pos.y = origin->pos.y;
		backup->pos.z = origin->pos.z;

		backup->right.x = origin->right.x;
		backup->right.y = origin->right.y;
		backup->right.z = origin->right.z;

		backup->up.x = origin->up.x;
		backup->up.y = origin->up.y;
		backup->up.z = origin->up.z;

		FRAME_EXTENSION(frame)->origMatrix = backup;
		return true;
	}
	else
	{
		if (useLog) lg << "Error: Unable to init new matrix backup A\n";
		if (useLog) lg.flush();
		FRAME_EXTENSION(frame)->origMatrix = nullptr;
		return false;
	}
}

