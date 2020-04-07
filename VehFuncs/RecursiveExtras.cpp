#include "VehFuncsCommon.h"
#include "Characteristics.h"
#include "RecursiveExtras.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CTheZones.h"
#include "CVisibilityPlugins.h"
#include "CWeather.h"
#include "CClock.h"
#include "CPopCycle.h"
#include "CPopulation.h"

///////////////////////////////////////////// Process Classes


int FindNextInterrogationMark(const string str, int from, int len)
{
	while (from < len)
	{
		from++;
		if (str[from] == '?') return from;
	} 
	return len;
}

bool ClassConditionsValid(const string nodeName, int from, CVehicle *vehicle)
{
	int next = 0;
	CZone *zone;
	string str;
	char zoneName[8];
	int len = nodeName.length();

	// For optimzation: faster and common functions before, slower ones at bottom
	while (from < len)
	{
		from++;

		// city
		if (nodeName[from] == 'c')
		{
			from++;
			int i = nodeName[from] - '0';
			if (CWeather::WeatherRegion == i) return true;
			from++;
		}

		// popcycle
		if (nodeName[from] == 'p')
		{
			from++;
			if (CPopCycle::m_nCurrentZoneType == stoi(&nodeName[from])) return true;
			from++;
		}

		// rain
		if (nodeName[from] == 'r' && nodeName[from + 1] == 'a' && nodeName[from + 2] == 'i' && nodeName[from + 3] == 'n') 
		{
			if (CWeather::OldWeatherType == 8 || CWeather::OldWeatherType == 16 ||
				CWeather::NewWeatherType == 8 || CWeather::NewWeatherType == 16 ||
				CWeather::ForcedWeatherType == 8 || CWeather::ForcedWeatherType == 16)
			{
				//lg << "RAIN OK" << endl;
				return true;
			}
			//else lg << "NOT RAIN" << endl;
			from += 4;
		}

		// norain
		if (nodeName[from] == 'n' && nodeName[from + 1] == 'o' && nodeName[from + 2] == 'r' && nodeName[from + 3] == 'a' && nodeName[from + 4] == 'i' && nodeName[from + 5] == 'n')
		{
			if (CWeather::OldWeatherType != 8 && CWeather::OldWeatherType != 16 &&
				CWeather::NewWeatherType != 8 && CWeather::NewWeatherType != 16 &&
				CWeather::ForcedWeatherType != 8 && CWeather::ForcedWeatherType != 16)
			{
				//lg << "NOT RAIN OK" << endl;
				return true;
			}
			//else lg << "NOT NOT RAIN" << endl;
			from += 6;
		}

		// hour
		if (nodeName[from] == 'h') 
		{
			from++;
			//lg << "curpopcycle " << CPopCycle::m_nCurrentZoneType << endl;
			int minHour;
			int maxHour;

			minHour = stoi(&nodeName[from]);
			if (minHour > 9)
				from += 1;
			else
				from += 2;
			maxHour = stoi(&nodeName[from]);

			//lg << minHour << " " << maxHour << endl;

			if (maxHour < minHour)
			{
				if (CClock::ms_nGameClockHours >= minHour || CClock::ms_nGameClockHours <= maxHour) return true;
			}
			else
			{
				if (CClock::ms_nGameClockHours >= minHour && CClock::ms_nGameClockHours <= maxHour) return true;
			}

			from++;
		}

		// zone
		if (nodeName[from] == 'z')
		{
			from++;
			next = FindNextInterrogationMark(nodeName, from, len);
			str = nodeName.substr(from, next);
			memset(zoneName, 0, 8);
			strncpy(zoneName, &str[0], next - from);
			//lg << "zone value " << zoneName << endl;
			from = next;
			if (CTheZones::FindZone(&vehicle->GetPosition(), *(int*)zoneName, *(int*)(zoneName + 4), eZoneType::ZONE_TYPE_NAVI))
			{
				//lg << "ZONE OK" << endl;
				return true;
			}
			//else lg << "NOT ZONE" << endl;
		}
	}
	return false;
}

void ProcessClassesRecursive(RwFrame * frame, CVehicle * vehicle, bool bReSearch)
{
	ExtendedData &xdata = xData.Get(vehicle);

	const string name = GetFrameNodeName(frame);
	size_t found;

	vector<RwFrame*> classNodes;
	vector<int> classNodesPercent;

	found = name.find("_reset");
	if (found != string::npos)
	{
		lg << "Extras: Reseting classes \n";
		list<string> &classList = getClassList();
		classList.clear();
	}

	RwFrame * tempNode = frame->child;
	if (tempNode) 
	{
		srand((xdata.randomSeed + xdata.randomSeedUsage));
		xdata.randomSeedUsage++;

		// Get properties
		int selectVariations = 1;
		bool randomizeVariations = false;

		size_t found = name.find(":");
		if (found != string::npos)
		{
			if (name.length() > found)
			{
				selectVariations = stoi(&name[found + 1]);
				int i = 0;
				do {
					if (name.length() > (found + 1 + i))
					{
						if (name[found + 2 + i] == '+') randomizeVariations = true;
					}
					i++;
				} while (i <= 1);
			}
		}

		// Count classes
		bool isAnyConditionClass = false;
		int random = 0;
		int totalClass = 0;
		while (tempNode) 
		{
			const string tempNodeName = GetFrameNodeName(tempNode);
			if (tempNodeName[0] == '!')
			{
				RwFrame *tempCharacFrame = tempNode->child;
				while (tempCharacFrame)
				{
					FindVehicleCharacteristicsFromNode(tempCharacFrame, vehicle, bReSearch);
					tempCharacFrame = tempCharacFrame->next;
				}
				tempNode = tempNode->next;
				continue;
			}

			found = tempNodeName.find_first_of("?");
			if (found != string::npos)
			{
				lg << "Found '?' condition at '" << tempNodeName << "'\n";
				if (!ClassConditionsValid(tempNodeName, found, vehicle)) {
					tempNode = tempNode->next;
					lg << "Condition check failed" << endl;
					continue;
				}
				else {
					found = tempNodeName.find("[");
					if (found != string::npos)
					{
						int percent = stoi(&tempNodeName[found + 1]);
						if (percent != 100)
						{
							int randomPercent = Random(1, 100);
							if (percent < randomPercent)
							{
								lg << "Class condition: " << tempNodeName << " not added due to percent\n";
								tempNode = tempNode->next;
								continue;
							}
						}
					}
					if (!isAnyConditionClass) {
						// Clear all other classes, only consider condition classes
						if (classNodes.size() > 0)
						{
							classNodes.clear();
							classNodesPercent.clear();
						}
						isAnyConditionClass = true;
					}
					classNodesPercent.push_back(100);
					classNodes.push_back(tempNode);
				}
			}
			else {
				// Don't add new classes if there is any condition class
				if (!isAnyConditionClass) {
					found = tempNodeName.find("[");
					if (found != string::npos)
					{
						int percent = stoi(&tempNodeName[found + 1]);
						classNodesPercent.push_back(percent);
					}
					else
					{
						classNodesPercent.push_back(100);
					}
					classNodes.push_back(tempNode);
					totalClass++;
				}
			}
			tempNode = tempNode->next;
			continue;
		}

		if (totalClass > 0) 
		{
			// Randomize (:+) or just fix max
			if (randomizeVariations == true)
				selectVariations = Random(selectVariations, totalClass);
			else
				if (selectVariations > totalClass) selectVariations = totalClass;

			// Insert classes
			int i = 0;
			do {
				random = Random(0, (totalClass - 1));

				if (classNodes[random] == nullptr) 
				{
					continue;
				}
				else
				{
					if (classNodesPercent[random] != 100)
					{
						int randomPercent = Random(1, 100);
						if (classNodesPercent[random] < randomPercent)
						{
							//lg << "Extras: Class not selected: " << classNodesPercent[random] << " percent\n";
							continue;
						}
					}

					FindVehicleCharacteristicsFromNode(classNodes[random], vehicle, bReSearch);
					if (classNodes[random]->child)
					{
						ProcessClassesRecursive(classNodes[random], vehicle, bReSearch);
					}

					string className = GetFrameNodeName(classNodes[random]);

					list<string> &classList = getClassList();

					size_t found1 = className.find_first_of("_");
					size_t found2 = className.find_first_of("[");
					size_t found3 = className.find_first_of(":");
					size_t found4 = className.find_first_of("?");
					if (found1 != string::npos || found2 != string::npos || found3 != string::npos || found4 != string::npos)
					{
						size_t cutPos;

						if (found1 < found2)
							cutPos = found1;
						else
							cutPos = found2;

						if (found3 < cutPos)
							cutPos = found3;

						if (found4 < cutPos)
							cutPos = found4;

						string classNameFixed = className.substr(0, cutPos);
						if (classNameFixed.length() > 0)
						{
							classList.push_back(classNameFixed);
							lg << "Extras: Class inserted: " << classNameFixed << "\n";
						}
					}
					else 
					{
						classList.push_back(className);
						lg << "Extras: Class inserted: " << className << "\n";
					}

					classNodes[random] = nullptr;
				}
				xdata.classFrame = classNodes[random];
				i++;
			} while (i < selectVariations);
		}
	}
}

///////////////////////////////////////////// Process Extras

void ProcessExtraRecursive(RwFrame * frame, CVehicle * vehicle) 
{
	const string name = GetFrameNodeName(frame);
	lg << "Extras: Processing node: '" << name << "'\n";
	
	RwFrame * tempFrame = frame->child;
	if (tempFrame != nullptr)
	{

		// -- Get properties
		int selectVariations = -1;
		bool randomizeVariations = false;

		size_t found = name.find(":");
		if (found != string::npos) 
		{
			if (name.length() > found) 
			{
				selectVariations = stoi(&name[found + 1]);
				int i = 0;
				do {
					if (name.length() > (found + 1 + i)) 
					{
						if (name[found + 2 + i] == '+') 
						{
							randomizeVariations = true;
						}
					}
					i++;
				} while (i <= 1);
			}
		}


		// -- Store extra frames
		int frames = 0;
		int classFrames = 0;
		bool hasClassFrame = false; // for performance
		RwFrame * extraFrames[33];
		RwFrame * extraFramesMatchClass[33];
		memset(extraFrames, 0, sizeof(extraFrames));
		memset(extraFramesMatchClass, 0, sizeof(extraFramesMatchClass));


		// -- List all extra frames
		while (frames < 32) 
		{
			if (tempFrame != nullptr) 
			{
				// Store class match frames
				list<string> &classList = getClassList();
				if (classList.size() > 0) 
				{
					// If frame has class
					string tempFrameName = GetFrameNodeName(tempFrame);
					size_t found = tempFrameName.find("[");
					if (found != string::npos) 
					{
						// Check name match
						for (list<string>::iterator it = classList.begin(); it != classList.end(); ++it) 
						{
							size_t found = tempFrameName.find(*it);
							if (found != string::npos) 
							{
								extraFramesMatchClass[classFrames] = tempFrame;
								lg << "Extras: Added to class list: " << tempFrameName << "\n";
								classFrames++;
							}
						}
					}
				}
				extraFrames[frames] = tempFrame;
				frames++;
			}
			else break;
			tempFrame = tempFrame->next;
		}


		// -- Randomize (:+) or just fix max
		if (randomizeVariations == true)
			selectVariations = Random(selectVariations, frames);
		else
			if (selectVariations > frames) selectVariations = frames;


		// -- Terminator for faster 'DeleteAllExtraFramesFromArray'
		extraFrames[(frames + 1)] = (RwFrame *)1;
		extraFramesMatchClass[(classFrames + 1)] = (RwFrame *)1;


		// -- Show has class
		lg << "Extras: Childs found: " << frames << "\n";
		if (classFrames > 0) 
		{
			lg << "Extras: Class childs found: " << classFrames << "\n";
			hasClassFrame = true;
		}


		// -- Num variations
		if (selectVariations == -1) 
		{
			if (frames == 1) selectVariations = 0;
			else selectVariations = 1;
		}
		lg << "Extras: Select variations: " << selectVariations << "\n";



		// -- Select random extra(s)
		int i = 0;
		int random = 0;
		do {
			if (selectVariations == 0)  // if ":0" or : was not set and just 1 frame = 50% chance to not appear
			{ 
				if (!Random(0, frames)) break;
			}
			if (classFrames > 0)  // from class frames array
			{ 
				random = Random(0, classFrames);
				if (extraFramesMatchClass[random] != nullptr)
				{
					i++;
					lg << "Extras: Starting new node (class match) " << i << " of " << selectVariations << " \n";
					ProcessExtraRecursive(extraFramesMatchClass[random], vehicle);

					RemoveFrameClassFromNormalArray(extraFramesMatchClass[random], extraFrames);
					extraFramesMatchClass[random] = nullptr;

					classFrames--;
				}
			}
			else  // from normal frames array
			{ 
				if (frames > 1) random = Random(0, (frames - 1));
				if (extraFrames[random] != nullptr)
				{
					if (FrameIsOtherClass(extraFrames[random]))
					{
						DestroyNodeHierarchyRecursive(extraFrames[random]);
						extraFrames[random] = nullptr;
						continue;
					}
					i++;
					lg << "Extras: Starting new node " << i << " of " << selectVariations << " \n";
					ProcessExtraRecursive(extraFrames[random], vehicle);

					extraFrames[random] = nullptr;
				}
				else
				{
					int k = 0;
					for (k = 0; k < frames; k++)
					{
						if (extraFrames[k] != nullptr) break;
					}
					if (k == frames)
					{
						lg << "Extras: There is no more extras to select \n";
						break;
					}
				}
			}
		} while (i < selectVariations);


		// -- Delete all not selected nodes
		for (int i = 0; i < 33; i++) 
		{
			if (extraFrames[i] == (RwFrame *)1) break; // Terminator
			if (extraFrames[i] != nullptr && extraFrames[i] != 0) 
			{
				DestroyNodeHierarchyRecursive(extraFrames[i]);
			}
		}

	}
	else lg << "Extras: Node has no child \n";

	return;
}

bool IsDamAtomic(RpAtomic *a)
{
	uint8_t *b = reinterpret_cast<uint8_t*>(a);
	int atomicPluginOffset = *reinterpret_cast<int*>(0x008D608C);
	uint16_t flags = *reinterpret_cast<uint16_t*>(b + atomicPluginOffset + 2);
	return (flags & 2) != 0;
}

bool IsOkAtomic(RpAtomic *a)
{
	uint8_t *b = reinterpret_cast<uint8_t*>(a);
	int atomicPluginOffset = *reinterpret_cast<int*>(0x008D608C);
	uint16_t flags = *reinterpret_cast<uint16_t*>(b + atomicPluginOffset + 2);
	return (flags & 1) != 0;
}

void RemoveFrameClassFromNormalArray(RwFrame * frameClass, RwFrame * frameArray[])
{
	for (int i = 0; i < 32; i++)
	{
		if (frameArray[i] == frameClass) { frameArray[i] = nullptr; return; }
	}
	return;
}


bool FrameIsOtherClass(RwFrame * frame)
{
	const string name = GetFrameNodeName(frame);
	size_t found = name.find("[");
	if (found != string::npos) 
	{
		list<string> &classList = getClassList();
		if (classList.size() > 0) 
		{
			for (list<string>::iterator it = classList.begin(); it != classList.end(); ++it) 
			{
				size_t foundclass = name.find(*it);
				if (foundclass != string::npos) 
				{
					return false;
				}
			}
		}
		return true;
	}
	return false;
}
