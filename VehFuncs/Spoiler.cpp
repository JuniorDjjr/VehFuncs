#include "VehFuncsCommon.h"
#include "CVisibilityPlugins.h"
#include "AtomicsVisibility.h"

void ProcessSpoiler(CVehicle *vehicle, list<RwFrame*> frames, bool after)
{
	// Process hide spoiler for tuning
	if (!frames.empty())
	{
		bool visible = true;

		if (vehicle->GetUpgrade(6) > 0) //spoiler upgrade
		{
			visible = false;
		}
		else
		{
			if (after)
				visible = true;
			else
				return;
		}

		for (RwFrame *frame : frames)
		{
			if (frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle)
			{
				if (!visible)
				{
					HideAllNodesRecursive_Forced(frame, true);
				}
				else {
					ShowAllNodesRecursive_Forced(frame, true);
				}
			}
			else
			{
				frames.remove(frame);
			}
		}
	}
}