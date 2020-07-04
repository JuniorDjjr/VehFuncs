#pragma once
#include "plugin.h"

using namespace std;

class F_an
{
public:
	RwFrame *frame;
	float progress;
	int mode;
	int submode;
	bool opening;
	int nextTimeToOpen;

	F_an(RwFrame *newframe)
	{
		frame = newframe;
		progress = 0.0f;
		mode = 0;
		submode = 0;
		opening = false;
		nextTimeToOpen = 0;
	}
};

void ProcessAnims(CVehicle *vehicle, list<F_an*> items);
