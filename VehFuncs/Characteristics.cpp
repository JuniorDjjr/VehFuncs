#include "VehFuncsCommon.h"
#include "LoadModel.h"
#include "NodeName.h"
#include "CCarEnterExit.h"
#include "IndieVehHandlingsAPI.h"
#include "CPopulation.h"
#include "CModelInfo.h"
#include "CPedModelInfo.h"
#include "CGeneral.h"
#include "CStreaming.h"
#include "AtomicsVisibility.h"
#include <bitset>

// Disable warnings (caution!)
#pragma warning( disable : 4244 ) // data loss

extern uint32_t txdIndexStart;

///////////////////////////////////////////// Find Vehicle Characteristics

void FindVehicleCharacteristicsFromNode(RwFrame * frame, CVehicle * vehicle, bool bReSearch) 
{
	const string name = GetFrameNodeName(frame);
	size_t found;

	//if (useLog) lg << "Charac: Processing node: " << name << "\n";

	found = name.find("="); // For performance
	if (found != string::npos)
	{
		if (!bReSearch)
		{
			// Paintjob
			found = name.find("_pj=");
			if (found != string::npos)
			{
				int paintjob;
				bool preserveColor = false;
				int digit1 = name[found + 4] - '0';

				ExtendedData &xdata = xData.Get(vehicle);

				if (name[found + 5] == '-')
				{
					int digit2 = name[found + 6] - '0';

					srand((xdata.randomSeed + xdata.randomSeedUsage));
					xdata.randomSeedUsage++;

					paintjob = Random(digit1, digit2);
					if (useLog) lg << "Charac: Found 'pj' (paintjob). Calculated from '-' result '" << paintjob << "' at '" << name << "'\n";
					if (name[found + 7] == 'c') preserveColor = true;
				}
				else
				{
					if (name[found + 5] == 'c') preserveColor = true;
					paintjob = digit1;
					if (useLog) lg << "Charac: Found 'pj' (paintjob). Set '" << digit1 << "' at '" << name << "'\n";
				}

				xdata.paintjob = paintjob;
				xdata.flags.bPreservePaintjobColor = preserveColor;
			}

			// Colors
			found = name.find("_cl=");
			if (found != string::npos)
			{
				ExtendedData &xdata = xData.Get(vehicle);

				int j = 0;
				char s[32] = { 0 };
				stringstream ss(&name[found + 4]);
				while (ss.getline(s, sizeof(s), ',')) { try { int num = stoi(s);  xdata.color[j] = num; j++; } catch (const exception &) { xdata.color[j] = -1; j++; } }

				if (useLog) lg << "Charac: Found 'cl' (colors) " << xdata.color[0] << " " << xdata.color[1] << " " << xdata.color[2] << " " << xdata.color[3] << "\n";
			}

			// Driver
			found = name.find("_drv=");
			if (found != string::npos)
			{
				list<int> driverList;

				char s[32] = { 0 };
				stringstream ss(&name[found + 5]);
				while (ss.getline(s, sizeof(s), ',')) { try { int num = stoi(s);  driverList.push_back(num); } catch (const exception &) { break; } }

				ExtendedData &xdata = xData.Get(vehicle);

				srand((xdata.randomSeed + xdata.randomSeedUsage));
				xdata.randomSeedUsage++;

				int rand = Random(0, (driverList.size() - 1));

				list<int>::iterator it = driverList.begin();
				advance(it, rand);
				int driverModel = *it;

				if (useLog) lg << "Charac: Found 'drv' (driver), selected '" << driverModel << "' at '" << name << "'\n";
				xdata.driverModel = driverModel;
			}

			// Occupants
			found = name.find("_oc=");
			if (found != string::npos)
			{
				ExtendedData &xdata = xData.Get(vehicle);

				char s[32] = { 0 };
				stringstream ss(&name[found + 4]);
				while (ss.getline(s, sizeof(s), ',')) { try { int num = stoi(s);  xdata.occupantsModels.push_back(num); } catch (const exception &) { break; } }

				if (useLog) lg << "Charac: Found 'oc' (occupants), variations " << xdata.occupantsModels.size() << " at '" << name << "'\n";

				found = name.find(".");
				if (found != string::npos)
				{
					xdata.passAddChance = name[found + 1] - '0';
				}
			}

			// MDPM custom chances
			found = name.find("_mdpmnpc=");
			if (found != string::npos)
			{
				int i = 9;
				int num = stoi(&name[found + i]);

				i++;
				do {
					i++;
				} while (name[found + i] != ',');

				i++;
				float minVol = stof(&name[found + i]);
				i++;

				do {
					i++;
				} while (name[found + i] != '-');
				i++;

				float maxVol = stof(&name[found + i]);

				if (useLog) lg << "MDPM: Found '_mdpmnpc' (MDPM NPC) chances '" << num << " vol " << minVol << "-" << maxVol << "'\n";
				ExtendedData &xdata = xData.Get(vehicle);
				xdata.mdpmCustomChances = num;
				xdata.mdpmCustomMinVol = minVol;
				xdata.mdpmCustomMaxVol = maxVol;
			}

			// Dirty (drt=5-9)
			found = name.find("_drt=");
			if (found != string::npos)
			{
				float fdigit1;
				float fdigit2;
				float dirtyLevel;

				ExtendedData &xdata = xData.Get(vehicle);

				int digit1 = name[found + 5] - '0';

				fdigit1 = digit1 * 1.6666f;

				if (name[found + 6] == '-')
				{
					int digit2 = name[found + 7] - '0';
					fdigit2 = digit2 * 1.6666f;

					srand((xdata.randomSeed + xdata.randomSeedUsage));
					xdata.randomSeedUsage++;

					dirtyLevel = CGeneral::GetRandomNumberInRange(fdigit1, fdigit2);
					if (useLog) lg << "Charac: Found 'drt' (dirty). Calculated from '-' result '" << dirtyLevel << "' at '" << name << "'\n";
				}
				else
				{
					dirtyLevel = fdigit1;
					if (useLog) lg << "Charac: Found 'drt' (dirty). Set '" << dirtyLevel << "' at '" << name << "'\n";
				}

				xdata.dirtyLevel = dirtyLevel;
			}
		}

		// Double exhaust smoke
		found = name.find("_dexh=");
		if (found != string::npos)
		{
			if (useLog) lg << "Charac: Found 'dexh' (double exhaust) at '" << name << "'\n";
			ExtendedData &xdata = xData.Get(vehicle);
			int enable = name[found + 6] - '0';
			if (enable) xdata.doubleExhaust = true;
			else xdata.doubleExhaust = false;
		}

		// Body tilt (not finished)
		/*found = name.find("_btilt=");
		if (found != string::npos)
		{
			if (useLog) lg << "Charac: Found 'btilt' (body tilt) at '" << name << "'\n";
			ExtendedData &xdata = xData.Get(vehicle);
			xdata.bodyTilt = stof(&name[found + 7]);
		}*/

	}

	// LOD less
	found = name.find("<");
	if (found != string::npos)
	{
		ExtendedData &xdata = xData.Get(vehicle);
		int num = name[found + 1] - '0';
		if (num != 0) num *= -1;
		FRAME_EXTENSION(frame)->LODdist = num;
		//FrameRenderAlways(frame);
		if (useLog) lg << "LOD: Found '<' level " << num << " at '" << name << "'\n";
	}

	// LOD greater
	found = name.find(">");
	if (found != string::npos)
	{
		ExtendedData &xdata = xData.Get(vehicle);
		int num = name[found + 1] - '0';
		FRAME_EXTENSION(frame)->LODdist = num;
		FrameRenderAlways(frame);
		if (useLog) lg << "LOD: Found '>' level " << num << " at '" << name << "'\n";
	}

	// Patches: Bus render
	found = name.find("_busrender");
	if (found != string::npos)
	{
		if (useLog) lg << "Charac: Found 'busrender' (force bus render) at '" << name << "'\n";
		ExtendedData &xdata = xData.Get(vehicle);
		xdata.flags.bBusRender = true;

		// Make driver be rendered in bus if it's already inside it (created in same frame)
		CPed *busDriver = vehicle->m_pDriver;
		if (busDriver) { busDriver->m_nPedFlags.bRenderPedInCar = true; }
	}

	return;
}



///////////////////////////////////////////// Set Characteristics

/*void SetCharacteristicsInSetModel(CVehicle * vehicle, bool bReSearch)
{
	ExtendedData &xdata = xData.Get(vehicle);

	return;
}*/


void SetCharacteristicsInRender(CVehicle * vehicle, bool bReSearch)
{
	ExtendedData &xdata = xData.Get(vehicle);

	if (!bReSearch) 
	{
		// Colors
		if (xdata.color[0] >= 0) 
		{
			if (useLog) lg << "Charac: Applying color 1: " << xdata.color[0] << "\n";
			vehicle->m_nPrimaryColor = xdata.color[0];
		}
		if (xdata.color[1] >= 0) 
		{
			if (useLog) lg << "Charac: Applying color 2: " << xdata.color[1] << "\n";
			vehicle->m_nSecondaryColor = xdata.color[1];
		}
		if (xdata.color[2] >= 0) 
		{
			if (useLog) lg << "Charac: Applying color 3: " << xdata.color[2] << "\n";
			vehicle->m_nTertiaryColor = xdata.color[2];
		}
		if (xdata.color[3] >= 0) 
		{
			if (useLog) lg << "Charac: Applying color 4: " << xdata.color[3] << "\n";
			vehicle->m_nQuaternaryColor = xdata.color[3];
		}

		// Paintjob
		if (xdata.paintjob >= 0) 
		{
			if ((int)xdata.paintjob > 0) {
				if (useLog) lg << "Charac: Applying paintjob: " << (int)xdata.paintjob << " keep color " << (bool)xdata.flags.bPreservePaintjobColor << "\n";
				vehicle->m_nVehicleFlags.bDontSetColourWhenRemapping = xdata.flags.bPreservePaintjobColor;
				vehicle->SetRemap(xdata.paintjob - 1);
				if (vehicle->m_nRemapTxd >= 0)
				{
					CStreaming::RequestModel(vehicle->m_nRemapTxd + txdIndexStart, eStreamingFlags::KEEP_IN_MEMORY | eStreamingFlags::PRIORITY_REQUEST);
					CStreaming::LoadAllRequestedModels(true);
				}
			}
		}

		// Dirty
		if (xdata.dirtyLevel >= 0.0) 
		{
			if (useLog) lg << "Charac: Applying dirty: " << xdata.dirtyLevel << "\n";
			vehicle->m_fDirtLevel = xdata.dirtyLevel;
		}

		// For only random created vehicles
		if (vehicle->m_nCreatedBy == eVehicleCreatedBy::RANDOM_VEHICLE) 
		{
			// Custom Driver
			if (xdata.driverModel > 0) 
			{
				CPed * driver = vehicle->m_pDriver;
				if (driver) 
				{
					char *a = (char*)driver;
					if (a[0x484] == 1) //Createdby RANDOM
					{ 
						int driverModel = xdata.driverModel;
						if (useLog) lg << "Charac: Changing driver: " << driverModel << "\n";
						if (LoadModel(driverModel)) 
						{
							CPedModelInfo *modelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(driverModel);
							driver->m_nPedType = (ePedType)modelInfo->m_nPedType;
							driver->SetModelIndex(driverModel);
							CCarEnterExit::SetPedInCarDirect(driver, vehicle, 0, true);
						}
						else
						{
							if (useLog) lg << "ERROR: Model doesn't exist! " << driverModel << "\n";
						}
					}
				}
			}

			// Custom Passengers
			if (!xdata.occupantsModels.empty()) 
			{
				CPed *driver = vehicle->m_pDriver;
				if (driver) 
				{
					char *a = (char*)driver;
					if (a[0x484] == 1) //Createdby RANDOM
					{ 
						if (xdata.driverModel <= 0) 
						{
							srand((xdata.randomSeed + xdata.randomSeedUsage));
							xdata.randomSeedUsage++;

							int rand = Random(0, (xdata.occupantsModels.size() - 1));

							list<int>::iterator it = xdata.occupantsModels.begin();
							advance(it, rand);
							int model = *it;

							if (useLog) lg << "Charac: Changing driver from '_oc': " << model << "\n";

							if (LoadModel(model)) 
							{
								CPedModelInfo *modelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(model);
								driver->m_nPedType = (ePedType)modelInfo->m_nPedType;
								driver->SetModelIndex(model);
								CCarEnterExit::SetPedInCarDirect(driver, vehicle, 0, true);
							}
							else
							{
								if (useLog) lg << "ERROR: Model doesn't exist! " << model << "\n";
							}
						}

						int maxPassengers = vehicle->m_nMaxPassengers;
						bool isTaxi = false;
						if (maxPassengers > 1) {
							if (xdata.taxiSignMaterial || vehicle->m_nModelIndex == eModelID::MODEL_TAXI || vehicle->m_nModelIndex == eModelID::MODEL_CABBIE) {
								isTaxi = true;
							}
						}
						for (int i = 0; i < maxPassengers; i++) 
						{
							CPed *pass = vehicle->m_apPassengers[i];
							if (!pass)
							{
								int rand = CGeneral::GetRandomNumberInRange(0, 10);
								int chances = xdata.passAddChance;
								if (maxPassengers > 1) {
									if (isTaxi) {
										if (i == 0) continue; // don't create front seat if taxi
									}
									else { // not taxi, increase chances for front seat
										if (i == 0) chances *= 2;
										else chances /= 2;
									}
								}
								if (rand > chances)
								{
									//if (useLog) lg << "Charac: No passenger added because of chance " << (int)xdata.passAddChance << "\n";
									continue;
								}
							}
							else 
							{
								char *a = (char*)pass;
								if (a[0x484] != 1) //Createdby NOT RANDOM
								{ 
									//if (useLog) lg << "Charac: No passenger changed: not random created\n";
									continue;
								}
							}

							int model = 0;
							int tries = 0;
							do {
								int rand = Random(0, (xdata.occupantsModels.size() - 1));

								list<int>::iterator it = xdata.occupantsModels.begin();
								advance(it, rand);
								model = *it;

								if (i != 0) break; // Don't do this for back seats

								tries++;
								if (tries > 5) break;
								
							} while (model == driver->m_nModelIndex);

							if (LoadModel(model))
							{
								CPedModelInfo *modelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(model);
								if (pass) 
								{
									if (useLog) lg << "Charac: Changing passenger " << (i + 1) << " model " << model << "\n";
									pass->SetModelIndex(model);
									pass->m_nPedType = (ePedType)modelInfo->m_nPedType;
								}
								else
								{
									if (useLog) lg << "Charac: Adding passenger " << (i + 1) << " model " << model << "\n";
									CVector pos;
									pos.x = vehicle->m_placement.m_vPosn.x;
									pos.y = vehicle->m_placement.m_vPosn.y;
									pos.z = vehicle->m_placement.m_vPosn.z;
									pass = CPopulation::AddPed((ePedType)modelInfo->m_nPedType, model, pos, 0);
								}
								int doorNodeId = CCarEnterExit::ComputeTargetDoorToEnterAsPassenger(vehicle, i);
								CCarEnterExit::SetPedInCarDirect(pass, vehicle, doorNodeId, false);
							}
							else
							{
								if (useLog) lg << "ERROR: Model doesn't exist! " << model << "\n";
							}
						}
					}
				}
			}
		}

	}

}

void SetCharacteristicsForIndieHandling(CVehicle * vehicle, bool bReSearch)
{
	ExtendedData &xdata = xData.Get(vehicle);

	if (!bReSearch)
	{
		// Double exhaust
		if (xdata.doubleExhaust >= 0) 
		{
			if (useLog) lg << "Charac: Applying double exhaust: " << xdata.doubleExhaust << "\n";
			tHandlingData * handling;
			if (IsIndieHandling(vehicle, &handling)) handling->m_bDoubleExhaust = xdata.doubleExhaust;
			else if (useLog) lg << "(ERROR) This function need IndieVehicles.asi installed \n";
		}
	}

	return;
}
