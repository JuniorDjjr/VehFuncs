#include "VehFuncsCommon.h"
#include "Characteristics.h"
#include "RecursiveExtras.h"
#include "Utilities.h"
#include "NodeName.h"
#include "CVisibilityPlugins.h"

///////////////////////////////////////////// Process Classes

void ProcessClassesRecursive(RwFrame * frame, CVehicle * vehicle, bool bReSearch)
{
	ExtendedData &xdata = remInfo.Get(vehicle);

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
			classNodes.push_back(tempNode);
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
			totalClass++;
			tempNode = tempNode->next;
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
					int randomPercent = Random(1, 100);
					if (classNodesPercent[random] < randomPercent)
					{
						//lg << "Extras: Class not selected: " << classNodesPercent[random] << " percent\n";
						continue;
					}

					FindVehicleCharacteristicsFromNode(classNodes[random], vehicle, bReSearch);
					if (classNodes[random]->child)
					{
						ProcessClassesRecursive(classNodes[random], vehicle, bReSearch);
					}

					string className = GetFrameNodeName(classNodes[random]);

					list<string> &classList = getClassList();

					size_t found1 = className.find("_");
					size_t found2 = className.find("[");
					size_t found3 = className.find(":");
					if (found1 != string::npos || found2 != string::npos || found3 != string::npos) 
					{
						size_t cutPos;

						if (found1 < found2)
							cutPos = found1;
						else
							cutPos = found2;

						if (found3 < cutPos)
							cutPos = found3;

						string classNameFixed = className.substr(0, cutPos);

						classList.push_back(classNameFixed);
						lg << "Extras: Class inserted: " << classNameFixed << "\n";
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
