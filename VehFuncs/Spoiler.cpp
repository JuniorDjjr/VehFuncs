#include "VehFuncsCommon.h"
#include "CVisibilityPlugins.h"
#include "AtomicsVisibility.h"

void ProcessSpoiler(CVehicle *vehicle, const list<RwFrame*> &frames, bool after)
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
			// Nothing to do for a dead node. This used to call frames.remove(),
			// which only ever hit the by-value copy of the list - so it removed
			// nothing - while invalidating the iterator this loop was using.
		}
	}
}