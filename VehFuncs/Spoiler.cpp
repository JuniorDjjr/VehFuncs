#include "VehFuncsCommon.h"
#include "CVisibilityPlugins.h"
#include "AtomicsVisibility.h"

void ProcessSpoiler(CVehicle *vehicle, list<RwFrame*> items, bool after)
{
	// Process hide spoiler for tuning
	if (!items.empty())
	{
		bool visible = true;
		bool forceHide = false;

		if (vehicle->GetUpgrade(6) > 0) //spoiler upgrade
		{
			forceHide = true;
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
				if (forceHide)
				{
					RpAtomic * atomic = (RpAtomic*)GetFirstObject(frame);
					if (CVisibilityPlugins::GetAtomicId(atomic) & 2)
					{
						if (reinterpret_cast<CAutomobile*>(vehicle)->m_damageManager.GetDoorStatus(eDoors::BOOT)) // is damaged
						{
							SetFrameFirstAtomicVisibility(frame, false); // force hide damaged
							continue;
						}
					}
				}
				SetFrameFirstAtomicVisibilityButFixedDamage(frame, visible, vehicle);
			}
			else
			{
				items.remove(*it);
			}
		}
	}
}