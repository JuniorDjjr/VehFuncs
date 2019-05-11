#pragma once
#include "VehFuncsCommon.h"

class CustomSeed {
public:
	int pvehicle;
	int seed;

	CustomSeed(int veh, int sd) {
		pvehicle = veh;
		seed = sd;
	}

	void Delete()
	{
		delete this;
	}

	bool operator==(const CustomSeed& other) const {
		if (this->pvehicle == other.pvehicle) return true;
		return false;
	}
};

extern list<CustomSeed> &getCustomSeedList();
