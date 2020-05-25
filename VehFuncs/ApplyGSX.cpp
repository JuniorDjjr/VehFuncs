#include "VehFuncsCommon.h"
#include "ApplyGSX.h"
#include "GSXAPI.h"

using namespace plugin;

extern fstream lg;

void callback(const GSX::externalCallbackStructure *test)
{
	using namespace GSX;

	if (test->veh)
	{
		ExtendedData &xdata = xData.Get(test->veh);

		switch (test->status)
		{
			case GSX::LOAD_CAR:
			{
				if (dataToLoadExists(test->veh, "randomSeed"))
				{
					int * seed = (int *)getSavedData(test->veh, "randomSeed");

					// apply
					xdata.randomSeed = *seed;
				}
				if (dataToLoadExists(test->veh, "odometerKms"))
				{
					float * kms = (float *)getSavedData(test->veh, "odometerKms");

					// apply
					xdata.kms = *kms;
				}
				break;
			}

			case 1:
			{
				// save
				float kms = xdata.kms;
				setDataToSaveLater(test->veh, "randomSeed", 4, &xdata.randomSeed, true);
				setDataToSaveLater(test->veh, "odometerKms", 4, &kms, true);
				break;
			}

			default:
				break;
		}
	}
	return;
}


void ApplyGSX()
{
	addNotifyCallback(callback);
	return;
}