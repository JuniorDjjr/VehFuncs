#include "VehFuncsCommon.h"
#include "AtomicsVisibility.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CAutomobile.h"
#include "CVisibilityPlugins.h"
#include "Utilities.h"

///////////////////////////////////////////// Setup Odometer

void SetupDigitalOdometer(CVehicle * vehicle, RwFrame * frame) 
{
	if (useLog) lg << "DigitalOdometer: Setup...\n";

	ExtendedData &xdata = xData.Get(vehicle);

	int setLod = GetDefaultLodForInteriorMinor(vehicle);

	frame = frame->child;
	if (frame != nullptr) 
	{
		RwFrame * frameDigits = nullptr;
		RwFrame * frameDigitsRoot = nullptr;

		int i = 0;
		while (i < 10) 
		{
			const char * name = GetFrameNodeName(frame);

			if (isdigit(name[5])) {
				int digitIndex = name[5] - '0' - 1;
				xdata.odometerDigits[digitIndex] = frame;
				FRAME_EXTENSION(frame)->LODdist = setLod;
			}
			else {
				if (frameDigits == nullptr)
				{
					if (strstr(name, "digits"))
					{
						frameDigits = frame;
						frameDigitsRoot = frame;
						frameDigits = frameDigits->child;
						if (frameDigits == nullptr) {
							if (useLog) lg << "DigitalOdometer: (error) 'f_dodometer' has no digits. Game may crash, fuck you \n";
							return;
						}
					}
				}
			}

			i++;
			frame = frame->next;
			if (frame == nullptr) break;
		}

		// Set digits to frames
		RpAtomic * tempAtomic;
		RpAtomic * newAtomic;
		for (int i = 0; i < 10; i++)
		{
			tempAtomic = (RpAtomic *)GetFirstObject(frameDigits);
			for (int j = 10; j >= 0; j--)
			{
				if (xdata.odometerDigits[j]) {
					newAtomic = RpAtomicClone(tempAtomic);
					RpAtomicSetFrame(newAtomic, xdata.odometerDigits[j]);
					RpClumpAddAtomic(vehicle->m_pRwClump, newAtomic);
				}
			}
			frameDigits = frameDigits->next;
		}
		DestroyNodeHierarchyRecursive(frameDigitsRoot);
	}
	return;
}



///////////////////////////////////////////// Process Odometer

void ProcessDigitalOdometer(CVehicle * vehicle, RwFrame * frame)
{
	ExtendedData &xdata = xData.Get(vehicle);
	if (frame->object.parent && FRAME_EXTENSION(frame)->owner == vehicle)
	{
		if (!FRAME_EXTENSION(frame)->flags.bNeverRender)
		{
			int finalKms = (int)(xdata.kms * xdata.odometerMult);

			int numLimit = 0;
			for (int i = 0; i < 9; ++i)
			{
				if (xdata.odometerDigits[i]) {
					numLimit++;
				}
			}
			numLimit = pow(10, numLimit);
			if (finalKms >= numLimit) finalKms = numLimit - 1;

			for (int i = 0; i < 9; ++i)
			{
				if (xdata.odometerDigits[i]) {
					bool canHideZeroes = false;
					int digitNum;
					if (i == 0) digitNum = finalKms % 10;
					else
					{
						int power = pow(10, i);
						digitNum = (finalKms / power) % 10;
						if (power > finalKms) canHideZeroes = true;
					}

					if (digitNum == 0 && canHideZeroes && xdata.odometerHideZero) {
						HideAllAtomics(xdata.odometerDigits[i]);
					}
					else {
						HideAllAtomicsExcept(xdata.odometerDigits[i], abs(digitNum - 9));
					} 
				}
			}
		}
	}
	else
	{
		ExtendedData &xdata = xData.Get(vehicle);
		xdata.odometerFrame = nullptr;
	}

	return;
}