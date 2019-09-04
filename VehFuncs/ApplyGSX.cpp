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

					if (seed != 0)
					{
						// apply
						xdata.randomSeed = *seed;
						lg << "Load Data: " << *seed << "\n";
					}
				}
				break;
			}

			case 1:
			{
				// save
				int seed = xdata.randomSeed;
				setDataToSaveLater(test->veh, "randomSeed", 4, &xdata.randomSeed, true);
				lg << "Save Data: " << seed << "\n";
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