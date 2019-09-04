#include "VehFuncsCommon.h"
#include "AtomicsVisibility.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CAutomobile.h"
#include "CVisibilityPlugins.h"

///////////////////////////////////////////// Setup Speedo

void SetupDigitalSpeedo(CVehicle * vehicle, RwFrame * frame) 
{
	lg << "DigitalSpeedo: Setup...\n";

	ExtendedData &xdata = xData.Get(vehicle);

	frame = frame->child;
	if (frame != nullptr) 
	{
		int i = 0;
		RwFrame * frameDigits = nullptr;
		RwFrame * frameDigitsRoot = nullptr;

		while (i < 4) 
		{
			const char * name = GetFrameNodeName(frame);

			// Find 'digits'
			if (frameDigits == nullptr) 
			{
				if (strstr(name, "digits"))
				{
					frameDigits = frame;
					frameDigitsRoot = frame;
					frameDigits = frameDigits->child;
					if (frameDigits == nullptr) lg << "DigitalSpeedo: (error) 'f_digitalspeedo' has no childs. Game may crash, fuck you \n";
					i++;
					continue;
				}
			}

			// Find 'digit1'
			if (xdata.speedoDigits[0] == nullptr)
			{
				if (strstr(name, "digit3"))
				{
					xdata.speedoDigits[0] = frame;
					i++;
					continue;
				}
			}
			// Find 'digit2'
			if (xdata.speedoDigits[1] == nullptr)
			{
				if (strstr(name, "digit2")) 
				{
					xdata.speedoDigits[1] = frame;
					i++;
					continue;
				}
			}
			// Find 'digit3'
			if (xdata.speedoDigits[2] == nullptr)
			{
				if (strstr(name, "digit1")) 
				{
					xdata.speedoDigits[2] = frame;
					i++;
					continue;
				}
			}

			frame = frame->next;
			if (frame == nullptr) break;
		}

		if (i == 4) 
		{
			// Set digits to frames
			RpAtomic * tempAtomic;
			RpAtomic * newAtomic;
			for (int i = 0; i < 10; i++)
			{
				tempAtomic = (RpAtomic *)GetFirstObject(frameDigits);
				for (int j = 2; j >= 0; j--)
				{
					newAtomic = RpAtomicClone(tempAtomic);
					RpAtomicSetFrame(newAtomic, xdata.speedoDigits[j]);
					RpClumpAddAtomic(vehicle->m_pRwClump, newAtomic);
				}
				frameDigits = frameDigits->next;
			}
			DestroyNodeHierarchyRecursive(frameDigitsRoot);
		} else lg << "DigitalSpeedo: Error: Bad adapted \n";
	}
	return;
}



///////////////////////////////////////////// Process Speedo

void ProcessDigitalSpeedo(CVehicle * vehicle, RwFrame * frame)
{
	ExtendedData &xdata = xData.Get(vehicle);

	//lg << "DigitalSpeedo: Reseting...\n";

	RwFrame * frameDigit1 = xdata.speedoDigits[0];
	RwFrame * frameDigit2 = xdata.speedoDigits[1];
	RwFrame * frameDigit3 = xdata.speedoDigits[2];

	//CVector vectorSpeed = vehicle->m_vecMoveSpeed;
	//float fspeed = vectorSpeed.Magnitude() * 200.0;
	float fspeed = GetVehicleSpeedRealistic(vehicle);

	fspeed *= xdata.speedoMult;

	while (fspeed < 0.0) fspeed *= -1.0;

	//lg << "DigitalSpeedo: speed: " << fspeed << "\n";

	int speedDigit1 = 0, speedDigit2 = 0, speedDigit3 = 0, ispeed = 0;

	if (fspeed > 1.0 || fspeed < -1.0)
	{
		ispeed = (int)fspeed;
		if (ispeed > 999) ispeed = 999;

		string speedDigits = to_string(ispeed);

		int len = speedDigits.length();

		if (len == 1)
		{
			speedDigit1 = 0;
			speedDigit2 = 0;
			speedDigit3 = speedDigits[0] - '0'; // 00X
		}
		if (len == 2) 
		{
			speedDigit1 = 0;
			speedDigit2 = speedDigits[0] - '0'; // 0X0
			speedDigit3 = speedDigits[1] - '0'; // 00X
		}
		if (len >= 3) 
		{
			speedDigit1 = speedDigits[0] - '0'; // X00
			speedDigit2 = speedDigits[1] - '0'; // 0X0
			speedDigit3 = speedDigits[2] - '0'; // 00X
		}
	}
	//lg << "DigitalSpeedo: " << speedDigit1 << speedDigit2 << speedDigit3 << "\n";

	if (ispeed < 100)
	{
		if (ispeed < 10) 
		{
			HideAllAtomics(frameDigit1);
			HideAllAtomics(frameDigit2);
			HideAllAtomicsExcept(frameDigit3, abs(speedDigit3 - 9));
		}
		else 
		{
			HideAllAtomics(frameDigit1);
			HideAllAtomicsExcept(frameDigit2, abs(speedDigit2 - 9));
			HideAllAtomicsExcept(frameDigit3, abs(speedDigit3 - 9));
		}
	}
	else 
	{
		HideAllAtomicsExcept(frameDigit1, abs(speedDigit1 - 9));
		HideAllAtomicsExcept(frameDigit2, abs(speedDigit2 - 9));
		HideAllAtomicsExcept(frameDigit3, abs(speedDigit3 - 9));
	}

	return;
}