#pragma once
#include "plugin.h"

struct MatrixBackup
{
	RwV3d               right;
	RwV3d               up;
	RwUInt32            pad1;
	RwV3d               at;
	RwUInt32            pad2;
	RwV3d               pos;
	RwUInt32            pad3;
};

void RestoreMatrixBackup(RwMatrix* dest, MatrixBackup* backup);

MatrixBackup* CreateMatrixBackup(RwMatrix* origin);
