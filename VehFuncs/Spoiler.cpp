#include "VehFuncsCommon.h"
#include "CVisibilityPlugins.h"
#include "AtomicsVisibility.h"

void ProcessSpoiler(CVehicle *vehicle, list<RwFrame*> items, bool after)
{
	// Process hide spoiler for tuning
	if (!items.empty())
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

		for (list<RwFrame*>::iterator it = items.begin(); it != items.end(); ++it)
		{
			RwFrame * frame = *it;
			if (frame->object.parent)
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
				items.remove(*it);
			}
		}
	}
}