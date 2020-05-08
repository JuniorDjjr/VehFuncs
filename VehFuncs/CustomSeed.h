#pragma once
#include "VehFuncsCommon.h"

class CustomSeed
{
public:
	int pvehicle;
	int seed;
	unsigned int timeToDeleteOfNotFound;

	CustomSeed(int veh, int sd, unsigned int timeLimit)
	{
		pvehicle = veh;
		seed = sd;
		timeToDeleteOfNotFound = timeLimit;
	}

	void Delete()
	{
		delete this;
	}

	bool operator==(const CustomSeed& other) const
	{
		if (this->pvehicle == other.pvehicle) return true;
		return false;
	}
};

extern list<CustomSeed> &getCustomSeedList();
