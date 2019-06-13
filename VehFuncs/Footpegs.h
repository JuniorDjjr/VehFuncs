#pragma once
#include "plugin.h"

using namespace std;

class F_footpegs
{
public:
	RwFrame *frame;
	float progress;

	F_footpegs(RwFrame *newframe)
	{
		frame = newframe;
		progress = 0.0f;
	}
};

void ProcessFootpegs(CVehicle *vehicle, list<F_footpegs*> items, int mode);
