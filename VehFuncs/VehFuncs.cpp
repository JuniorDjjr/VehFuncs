/*
    Author: Junior_Djjr - MixMods.com.br
    Created using https://github.com/DK22Pac/plugin-sdk
*/
#include "VehFuncs.h"

// Mod utilities & API
#include "AtomicsVisibility.h"
#include "IndieVehHandlingsAPI.h"
#include "CustomSeed.h"
#include "Matrixbackup.h"
#include "Utilities.h"
//#include "CheckRepair.h"

// Mod funcs
#include "Patches.h"
#include "FixMaterials.h"
#include "DigitalSpeedo.h"
#include "Characteristics.h"
#include "RecursiveExtras.h"
#include "GearAndFan.h"
#include "Shake.h"
#include "Pedal.h"
#include "Footpegs.h"
#include "PopupLights.h"
#include "Trifork.h"
#include "Spoiler.h"
#include "Anims.h"
#include "Steer.h"
 
// Dependences
#include "../injector/assembly.hpp"
#include "extensions/ScriptCommands.h"
#include "CVisibilityPlugins.h"
#include "CTxdStore.h"
#include "CModelInfo.h"
#include "NodeName.h"
#include "CStreaming.h"
#include "CGeneral.h"
#include "CTask.h"
#include "CTimer.h"
#include "CVector.h"
#include <time.h>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <sstream>

// Disable warnings (caution!)
#pragma warning( disable : 4244 ) // data loss

// Global vars
int G_i = 0;
uintptr_t AtomicAlphaCallBack;
uint32_t txdIndexStart;
VehicleExtendedData<ExtendedData> xData;
fstream lg;
bool IVFinstalled = false, APPinstalled = false, bFirstFrame = false, bFirstScriptFrame = false;
CVehicle *curVehicle;
extern RwTexDictionary *vehicletxdArray[4];
extern int vehicletxdIndexArray[4];
std::list<std::pair<unsigned int *, unsigned int>> resetMats;

///////////////////////////////////////////////////////////////////////////////////////////////////


class VehFuncs
{
public:
	VehFuncs()
	{
		// -- On plugin init
		lg.open("VehFuncs.log", fstream::out | fstream::trunc);
		static bool reInit = false;
		xData = getExtData();

		// Fix for remap txd names. This also stores additional vehicle*.txd files.
		memset(vehicletxdArray, 0, sizeof(vehicletxdArray));
		memset(vehicletxdIndexArray, 0, sizeof(vehicletxdIndexArray));
		patch::RedirectCall(0x5B62C2, Patches::CustomAssignRemapTxd, true);

		// Preprocess hierarchy don't remove frames
		MakeJMP(0x004C8E30, CustomCollapseFramesCB);


		// -- On game init
		Events::initGameEvent += []
		{
			lg << "VF v1.8\n";

			srand(time(0));
			StoreHandlingData();
			ApplyGSX();
			//ApplyCheckRepair();
			
			AtomicAlphaCallBack = ReadMemory<int>(0x4C7842, false);
			txdIndexStart = ReadMemory<uint32_t>(0x6D65D2 + 1, true);


			// Fix "ug_" dummies outside "chassis" (ID 1) using first node (ID 0) (usually a wheel node).
			MakeInline<0x004C8FA1, 0x004C8FA1 + 6>([](reg_pack& regs)
			{
				if (regs.eax == 0) regs.eax = 1; // set frame ID 1 by default
				*(uint32_t*)(regs.esp + 0x64 - 0x34) = regs.eax;  //mov     [esp+64h+atomic2], eax ; frame visibility
				regs.eax = *(uint32_t*)regs.edx;  //mov     eax, [edx]
			});


			// LODs (make our custom LOD always render)
			MakeNOP(0x00733241, 6);
			MakeJMP(0x00733241, Patches::ForceRenderCustomLOD);
			MakeNOP(0x00733F80, 6);
			MakeJMP(0x00733F80, Patches::ForceRenderCustomLODAlpha);
			MakeNOP(0x007344A0, 6);
			MakeJMP(0x007344A0, Patches::ForceRenderCustomLODBoatAlpha);
			MakeNOP(0x00733550, 6);
			MakeJMP(0x00733550, Patches::ForceRenderCustomLODBoat);
			MakeNOP(0x00733420, 6);
			MakeJMP(0x00733420, Patches::ForceRenderCustomLODBig);
			MakeNOP(0x00734370, 6);
			MakeJMP(0x00734370, Patches::ForceRenderCustomLODBigAlpha);
			MakeNOP(0x00733331, 6);
			MakeJMP(0x00733331, Patches::ForceRenderCustomLODTrain);
			MakeNOP(0x00734240, 6);
			MakeJMP(0x00734240, Patches::ForceRenderCustomLODTrainAlpha);
			
			

			// Preprocess hierarchy find damage atomics to apply damageable
			MakeCALL(0x4C9173, Patches::FindDamage::CustomFindDamageAtomics, true);
			WriteMemory<uint32_t>(0x4C916D + 1, memory_pointer(Patches::FindDamage::CustomFindDamageAtomicsCB).as_int(), true);


			// Render bus driver
			MakeNOP(0x0064BCB3, 6);
			MakeCALL(0x0064BCB3, Patches::RenderBus);


			// Cop functions
			MakeJMP(0x006D2379, Patches::IsLawEnforcement);
			Patches::PatchForCoplights();


			// Popup lights
			MakeInline<0x006E1CD4, 0x006E1CD4 + 6>([](reg_pack& regs)
			{
				//mov eax, [esi+590h]
				regs.eax = *(uint32_t*)(regs.esi + 0x590);

				CVehicle *veh = (CVehicle*)regs.esi;
				CEntity *entity = (CEntity*)veh;
				ExtendedData &xdata = xData.Get(veh);

				if (xdata.popupFrame[0] != nullptr)
				{
					bool noLights = false;
					CAutomobile *automobile = (CAutomobile*)veh;

					if (automobile->m_damageManager.GetLightStatus(LIGHT_FRONT_LEFT))
					{
						if (xdata.popupProgress[LIGHT_FRONT_RIGHT] != 1.0f)
						{
							noLights = true;
						}
					}
					else if (automobile->m_damageManager.GetLightStatus(LIGHT_FRONT_RIGHT))
					{
						if (xdata.popupProgress[LIGHT_FRONT_LEFT] != 1.0f)
						{
							noLights = true;
						}
					}
					else
					{
						if (xdata.popupProgress[LIGHT_FRONT_LEFT] != 1.0f && xdata.popupProgress[LIGHT_FRONT_RIGHT] != 1.0f)
						{
							noLights = true;
						}
					}

					if (noLights) *(uint32_t*)(regs.esp - 0x4) = 0x6E28EF; //don't process lights
				}
			});


			// Upgrade updated
			MakeInline<0x006E32AB, 0x006E32AB + 8>([](reg_pack& regs)
			{
				*(uint32_t*)(regs.esp + 0x14) = -1; //mov     dword ptr [esp+14h], 0FFFFFFFFh

				CVehicle *veh = (CVehicle *)regs.edi;
				ExtendedData &xdata = xData.Get(veh);
				xdata.flags.bUpgradesUpdated = true;
			});

			MakeInline<0x006DF96C, 0x006DF96C + 6>([](reg_pack& regs)
			{
				regs.eax = *(uint16_t*)(regs.esi + 0x12); //mov     ax, [esi+12h]
				regs.esi = regs.eax; //mov     esi, eax

				CVehicle *veh = (CVehicle *)regs.edi;
				ExtendedData &xdata = xData.Get(veh);
				xdata.flags.bUpgradesUpdated = true;
			});


			// RpClumpRender
			MakeNOP(0x00749B3E, 9, true);
			MakeJMP(0x00749B3E, Patches::NeverRender);

			
			
			// Upgrade replace
			//WriteMemory<uint32_t>(0x6D383D, 0x000, true);
			// Add
			MakeCALL(0x006D386C, CustomRwFrameForAllChildren_AddUpgrade);
			// Remove
			MakeCALL(0x006D3A18, CustomRwFrameForAllChildren_RemoveUpgrade);
			// Add original
			MakeInline<0x006D3A35, 0x006D3A35 + 18>([](reg_pack& regs)
			{
				RwFrame *sourceFrame = (RwFrame *)regs.eax;
				RwFrame *destFrame = (RwFrame *)regs.edi;
				RpClump *destClump = (RpClump *)regs.edx;
				//CloneNode(sourceFrame, destClump, destFrame, true, true);
				*(uint32_t*)0xC1CB58 = (uint32_t)destClump;
				RwFrameForAllObjects(sourceFrame, CopyObjectsCB, destFrame);
			});
			WriteMemory<uint8_t>(0x6D3A47 + 2, 0x28, true);


			// Additional generic vehicle*.txd files
			Patches::PatchForAdditionalVehicleTxd();

			
			// Hitch patch
			// CAutomobile
			int *vmt = (int*)0x00871120;
			int *getlink = vmt + (0xF0 / sizeof(vmt));
			Patches::Hitch::setOriginalFun((Patches::Hitch::GetTowBarPos_t)ReadMemory<int*>(getlink, true));
			WriteMemory(getlink, memory_pointer(Patches::Hitch::GetTowBarPosToHook).as_int(), true);
			// CTrailer
			Patches::Hitch::setOriginalFun_Trailer((Patches::Hitch::GetTowBarPos_t)ReadMemory<int*>(0x871D18, true));
			WriteMemory(0x871D18, memory_pointer(Patches::Hitch::GetTowBarPosToHook).as_int(), true);

			lg << "Core: Started\n";
		};



		// -- On plugins attach
		Events::attachRwPluginsEvent += []() 
		{
			FramePluginOffset = RwFrameRegisterPlugin(sizeof(FramePlugin), PLUGIN_ID_STR, (RwPluginObjectConstructor)FramePlugin::Init, (RwPluginObjectDestructor)FramePlugin::Destroy, (RwPluginObjectCopy)FramePlugin::Copy);
		};



		// -- On game init
		Events::initGameEvent.after += []()
		{
			if (!bFirstFrame)
			{
				if (ReadMemory<uint32_t>(0x004C9148, true) != 0x004C8E30)
				{
					lg << "Core: IVF installed\n";
					WriteMemory(0x004C9148, (int)0x004C8E30, true); // unhook IVF collapse frames
					IVFinstalled = true;
				}
				else
				{
					lg << "Core: IVF not installed\n";
					IVFinstalled = false;
				}
				lg.flush();
				bFirstFrame = true;
			}
		};


		// -- On process script (gameProcessEvent isn't compatible with SAMP)
		Events::processScriptsEvent.after += [] {

			if (!bFirstScriptFrame)
			{
				if (GetModuleHandleA("CLEO.asi")) {
					unsigned int script;
					const unsigned int GET_SCRIPT_STRUCT_NAMED = 0x10AAA;
					Command<GET_SCRIPT_STRUCT_NAMED>("NEWSVAN", &script);
					if (script)
					{
						lg << "Core: AD installed\n";
						APPinstalled = true;
					}
					else
					{
						lg << "Core: AD not installed\n";
						APPinstalled = false;
					}
					
				} else lg << "Core: CLEO isn't installed." << "\n\n";
				lg.flush();
				bFirstScriptFrame = true;
			}
		};


		// -- On vehicle set model
		Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelId) 
		{
			ExtendedData &xdata = xData.Get(vehicle);
			xdata.nodesProcess = true;
			xdata.nodesProcessForIndieHandling = true;
			xdata.ReInitForReSearch();
			//lg << "Core: Model set\n";
		};



		// -- On vehicle render
		Events::vehicleRenderEvent.before += [](CVehicle *vehicle)
		{

			// Reset material stuff (after render) - doesn't work on .after, I don't know why...
			for (auto &p : resetMats)
				*p.first = p.second;
			resetMats.clear();

			// Init
			curVehicle = vehicle;
			ExtendedData &xdata = xData.Get(vehicle);
			tHandlingData *handling;
			bool bReSearch = false;

			// Set custom seed
			list<CustomSeed> &customSeedList = getCustomSeedList();
			if (customSeedList.size() > 0)
			{
				lg << "Custom Seed: Running custom seed list \n";

				for (list<CustomSeed>::iterator it = customSeedList.begin(); it != customSeedList.end(); ++it)
				{
					CustomSeed customSeed = *it;

					if (reinterpret_cast<int>(vehicle) == customSeed.pvehicle)
					{
						lg << "Custom Seed: Seed " << customSeed.seed << " set to " << customSeed.pvehicle << "\n";
						xdata.randomSeed = customSeed.seed;
						customSeedList.remove(*it);

						if (customSeedList.size() <= 0) break;
					}
				}
			}


			// For IndieVehHandling / Get re-search
			bool isIndieHandling = true;
			if (IsIndieHandling(vehicle, &handling))
			{
				isIndieHandling = true;
				bReSearch = ExtraInfoBitReSearch(vehicle, handling);
			}

			// Search nodes
			if (xdata.nodesProcess || bReSearch)
			{
				if (bReSearch)
				{
					// Reset extended data
					xdata.ReInitForReSearch();
				}
				// Clear temp class list
				list<string> &classList = getClassList();
				classList.clear();

				// Process all nodes
				xdata.randomSeedUsage = 0;
				RwFrame *rootFrame = (RwFrame *)vehicle->m_pRwClump->object.parent;
				FindNodesRecursive(rootFrame, vehicle, bReSearch, false);

				if (!bReSearch)
				{
					// Set wheels
					if (xdata.wheelFrame[0]) SetWheel(xdata.wheelFrame, vehicle);

					// Fix materials
					FixMaterials(vehicle->m_pRwClump);
				}

				// Post set
				SetCharacteristicsInRender(vehicle, bReSearch);
				xdata.nodesProcess = false;


				// TEST
				/*
				RwStream *splate = RwStreamOpen(RwStreamType::rwSTREAMFILENAME, RwStreamAccessType::rwSTREAMREAD, "grass0_1.dff");
				RpClump *cplate;
				RpAtomic *aplate;

				if (RwStreamFindChunk(splate, 0x10, 0, 0)) {

					cplate = RpClumpStreamRead(splate);
					aplate = GetFirstAtomic(cplate);

					if (aplate) {

						CAutomobile *aveh = (CAutomobile*)vehicle;
						RwFrame *boot = aveh->m_aCarNodes[CAR_BONNET];

						if (boot) {

							RwFrame *newFrame = RwFrameCreate();
							RpAtomic *plateAtomic = RpAtomicClone(aplate);

							RpAtomicSetFrame(plateAtomic, newFrame);
							RpClumpAddAtomic(vehicle->m_pRwClump, plateAtomic);
							RwFrameAddChild(boot, newFrame);


							int slot = CTxdStore::AddTxdSlot("platetest");
							CTxdStore::LoadTxd(slot, "plant1.txd");
							CTxdStore::AddRef(slot);
							CTxdStore::SetCurrentTxd(slot);
							RwTexture *text = plugin::CallAndReturn<RwTexture*, 0x7F3AC0, const char*, const char*>("txgrass1_3", 0);
							CTxdStore::PopCurrentTxd();

							CAutomobile *aveh = (CAutomobile*)vehicle;
							RwFrame *boot = aveh->m_aCarNodes[CAR_BONNET];

							if (boot) {

								RwFrame *newFrame = RwFrameCreate();
								RpAtomic *plateAtomic = RpAtomicClone(aplate);

								RpAtomicSetFrame(plateAtomic, newFrame);
								RpClumpAddAtomic(vehicle->m_pRwClump, plateAtomic);
								RwFrameAddChild(boot, newFrame);

								RpGeometryLock(plateAtomic->geometry, 0xFFF);
								plateAtomic->geometry->flags = plateAtomic->geometry->flags & 0xFFFFFFCF | 0x40;
								RpGeometryUnlock(plateAtomic->geometry);

								int slot = CTxdStore::AddTxdSlot("platetest");
								CTxdStore::LoadTxd(slot, "plant1.txd");
								CTxdStore::AddRef(slot);
								CTxdStore::SetCurrentTxd(slot);
								RwTexture *text = plugin::CallAndReturn<RwTexture*, 0x7F3AC0, const char*, const char*>("txgrass1_3", 0);
								CTxdStore::PopCurrentTxd();

								lg << "Shared Extras Texture: " << text << "\n";
								fs.flush();

								text->filterAddressing = 2;

								if (text) {
									RwRGBA *rgb = new RwRGBA{ 255,255,255 };
									plateAtomic->geometry->matList.materials[0]->color = *rgb;
									plateAtomic->geometry->matList.materials[0]->texture = text;
								}

							}

						}
					}

				}
				RwStreamClose(splate, 0);

				lg << "Shared Extras: " << aplate << "\n";
				fs.flush();
			*/
			}

			if (isIndieHandling && (xdata.nodesProcessForIndieHandling || bReSearch))
			{
				SetCharacteristicsForIndieHandling(vehicle, bReSearch);
				xdata.nodesProcessForIndieHandling = false;
			}

			///////////////////////////////////////////////////////////////////////////////////////

			// Process store smooth pedal
			int gasSoundProgress = vehicle->m_vehicleAudio.field_14C;
			int rpmSound = vehicle->m_vehicleAudio.field_148;

			if (gasSoundProgress == 0 && vehicle->m_fMovingSpeed > 0.2f && rpmSound != -1)
			{ // fix me: the last gear (max speed) is ignored
				xdata.smoothGasPedal = 0.0f;
			}
			else
			{
				float gasPedal = abs(vehicle->m_fGasPedal);
				if (gasPedal > 0.0f)
				{
					xdata.smoothGasPedal += (CTimer::ms_fTimeStep / 1.6666f) * (gasPedal / 6.0f);
					if (xdata.smoothGasPedal > 1.0f) xdata.smoothGasPedal = 1.0f;
					else if (xdata.smoothGasPedal > gasPedal) xdata.smoothGasPedal = gasPedal;
				}
				else
				{
					if (xdata.smoothGasPedal > 0.0f)
					{
						xdata.smoothGasPedal -= (CTimer::ms_fTimeStep / 1.6666f) * 0.3;
						if (xdata.smoothGasPedal < 0.0f) xdata.smoothGasPedal = 0.0f;
					}
				}
			}

			float brakePedal = abs(vehicle->m_fBreakPedal);
			if (brakePedal > 0.0f)
			{
				xdata.smoothBrakePedal += (CTimer::ms_fTimeStep / 1.6666f) * (brakePedal / 6.0f);
				if (xdata.smoothBrakePedal > 1.0f) xdata.smoothBrakePedal = 1.0f;
				else if (xdata.smoothBrakePedal > brakePedal) xdata.smoothBrakePedal = brakePedal;
			}
			else
			{
				if (xdata.smoothBrakePedal > 0.0f)
				{
					xdata.smoothBrakePedal -= (CTimer::ms_fTimeStep / 1.6666f) * 0.3;
					if (xdata.smoothBrakePedal < 0.0f) xdata.smoothBrakePedal = 0.0f;
				}
			}

			///////////////////////////////////////////////////////////////////////////////////////

			// Process material stuff (before render)
			if (xdata.taxiSignMaterial)
			{
				if (reinterpret_cast<CAutomobile*>(vehicle)->taxiAvaliable & 1)
				{
					resetMats.push_back(std::make_pair(reinterpret_cast<unsigned int *>(&xdata.taxiSignMaterial->surfaceProps.ambient), *reinterpret_cast<unsigned int *>(&xdata.taxiSignMaterial->surfaceProps.ambient)));
					xdata.taxiSignMaterial->surfaceProps.ambient = 10.0f;
				}
			}

			// Process speedo
			if (xdata.speedoFrame != nullptr)
			{
				if (xdata.speedoDigits != nullptr)
				{
					ProcessDigitalSpeedo(vehicle, xdata.speedoFrame);
				}
			}

			if (vehicle->m_nVehicleFlags.bEngineOn)
			{
				// Process gear
				if (!xdata.gearFrame.empty()) ProcessRotatePart(vehicle, xdata.gearFrame, true);

				// Process fan
				if (!xdata.fanFrame.empty()) ProcessRotatePart(vehicle, xdata.fanFrame, false);
			}
			// Process shake
			if (!xdata.shakeFrame.empty()) ProcessShake(vehicle, xdata.shakeFrame);

			// Process gas pedal
			if (!xdata.gaspedalFrame.empty()) ProcessPedal(vehicle, xdata.gaspedalFrame, 1);

			// Process brake pedal
			if (!xdata.brakepedalFrame.empty()) ProcessPedal(vehicle, xdata.brakepedalFrame, 2);

			if (vehicle->m_fHealth > 0 && !vehicle->m_nVehicleFlags.bEngineBroken && !vehicle->m_nVehicleFlags.bIsDrowning)
			{
				// Process anims
				if (!xdata.anims.empty()) ProcessAnims(vehicle, xdata.anims);

				// Process popup lights
				if (xdata.popupFrame[0]) ProcessPopup(vehicle, &xdata);
			}

			// Process trifork
			if (xdata.triforkFrame) ProcessTrifork(vehicle, xdata.triforkFrame);

			// Process footpegs
			if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX)
			{
				if (!xdata.fpegFront.empty()) ProcessFootpegs(vehicle, xdata.fpegFront, 1);
				if (!xdata.fpegBack.empty()) ProcessFootpegs(vehicle, xdata.fpegBack, 2);
			}

			// Process tuning spoiler (before render)
			if (xdata.flags.bUpgradesUpdated)
			{
				ProcessSpoiler(vehicle, xdata.spoilerFrames, false);
			}

			// Process steer
			if (!xdata.steer.empty()) ProcessSteer(vehicle, xdata.steer);

		};

		///////////////////////////////////////////////////////////////////////////////////////

		Events::vehicleRenderEvent.after += [](CVehicle *vehicle)
		{
			curVehicle = vehicle;
			ExtendedData &xdata = xData.Get(vehicle);

			// Process tuning spoiler (after render)
			if (xdata.flags.bUpgradesUpdated)
			{
				ProcessSpoiler(vehicle, xdata.spoilerFrames, true);
			}

			// Post reset flags
			xdata.flags.bDamageUpdated = false;
			xdata.flags.bUpgradesUpdated = false;
		};

		///////////////////////////////////////////////////////////////////////////////////////

		// -- Flush log during unfocus (ie minimizing)
		Events::onPauseAllSounds += []
		{
			lg.flush();
		};

		/*
		// -- On game reload
		Events::reInitGameEvent += [] // It's also called first time the game is initialized
		{
			if (reInit)
			{
				lg << "Core: Reinitializing game...\n";
				lg.flush();
				// ...?
			}
			else reInit = true;
		};
		*/
    }

	///////////////////////////////////////////////////////////////////////////////////////////////




	///////////////////////////////////////////// Find Nodes

	static void FindNodesRecursive(RwFrame * frame, CVehicle * vehicle, bool bReSearch, bool bOnExtras)
	{
		while (frame) 
		{
			const string name = GetFrameNodeName(frame);
			size_t found;

			if (name[0] == 'f' && name[1] == '_') 
			{
				//lg << "Checking for " << name << endl;

				ExtendedData &xdata = xData.Get(vehicle);
				if (!bReSearch) 
				{
					// Don't process extras if seed is 0 (for Tuning Mod and other mods)
					if (xdata.randomSeed != 0)
					{
						// Set extras class
						found = name.find("f_class");
						if (found != string::npos) 
						{
							lg << "Extras: Found 'f_class' \n";

							ProcessClassesRecursive(frame, vehicle, bReSearch);

							if (RwFrame * tempFrame = frame->next) 
							{
								lg << "Extras: Jumping class nodes \n";
								frame = tempFrame;
								continue;
							}
						}
						
						// Recursive extras
						found = name.find("f_extras");
						if (found != string::npos) 
						{
							lg << "Extras: Found 'f_extras' \n";

							RwFrame * tempFrame = frame->child;
							if (tempFrame != nullptr) 
							{
								// Prepare for extra search
								srand((xdata.randomSeed + xdata.randomSeedUsage));
								xdata.randomSeedUsage++;

								list<string> &classList = getClassList();

								lg << "Extras: Classes: ";
								for (list<string>::iterator it = classList.begin(); it != classList.end(); ++it) {
									lg << *it << " ";
								}
								lg << "\n";

								lg << "Extras: --- Starting - veh ID " << vehicle->m_nModelIndex << "\n";
								ProcessExtraRecursive(frame, vehicle);
								lg << "Extras: --- Ending \n";
							}
							else lg << "Extras: (error) 'f_extras' has no childs \n";
						}
					}
				}

				// Digital speedo
				found = name.find("f_dspeedo");
				if (found != string::npos) 
				{
					lg << "DigitalSpeedo: Found 'f_dspeedo' \n";

					SetupDigitalSpeedo(vehicle, frame);
					xdata.speedoFrame = frame;
					FRAME_EXTENSION(frame)->owner = vehicle;

					float speedMult = 1.0;

					found = name.find("_mph");
					if (found != string::npos) 
					{
						speedMult *= 0.621371f;
					}

					found = name.find("_mu=");
					if (found != string::npos) 
					{
						float mult = stof(&name[found + 4]);
						speedMult *= mult;
					}

					lg << "DigitalSpeedo: Speed multiplicator: " << speedMult << "\n";

					xdata.speedoMult = speedMult;
				}

				// Gear
				found = name.find("f_gear");
				if (found != string::npos) 
				{
					lg << "Gear: Found 'f_gear' \n";
					xdata.gearFrame.push_back(frame);
					FRAME_EXTENSION(frame)->owner = vehicle;
				}

				// Fan
				found = name.find("f_fan");
				if (found != string::npos) 
				{
					lg << "Gear: Found 'f_fan' \n";
					xdata.fanFrame.push_back(frame);
					FRAME_EXTENSION(frame)->owner = vehicle;
				}

				// Shake
				found = name.find("f_shake");
				if (found != string::npos) 
				{
					lg << "Shake: Found 'f_shake' \n";
					if (CreateMatrixBackup(frame)) {
						xdata.shakeFrame.push_back(frame);
						FRAME_EXTENSION(frame)->owner = vehicle;
					}
				}

				// Gaspedal
				found = name.find("f_gas");
				if (found != string::npos) 
				{
					lg << "Pedal: Found 'f_gas' \n";
					if (CreateMatrixBackup(frame)) {
						xdata.gaspedalFrame.push_back(frame);
						FRAME_EXTENSION(frame)->owner = vehicle;
					}
				}

				// brakepedal
				found = name.find("f_brake");
				if (found != string::npos) 
				{
					lg << "Pedal: Found 'f_brake' \n";
					if (CreateMatrixBackup(frame)) {
						xdata.brakepedalFrame.push_back(frame);
						FRAME_EXTENSION(frame)->owner = vehicle;
					}
				}

				found = name.find("f_an"); //f_an1a=
				if (found != string::npos)
				{
					if (name[6] == '=')
					{
						int mode = stoi(&name[4]);
						int submode = (name[5] - 'a');

						switch (mode)
						{
						case 0:
							lg << "Anims: Found 'f_an" << mode << "': ping pong \n";
							break;
						case 1:
							switch (submode)
							{
							case 0:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": engine off \n";
								break;
							case 1:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": engine off or alarm on \n";
								break;
							default:
								lg << "Anims: Found 'f_an" << mode << " ERROR: submode not found \n";
								submode = -1;
								break;
							}
							break;
						case 2:
							switch (submode)
							{
							case 0:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": driver \n";
								break;
							case 1:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 1 \n";
								break;
							case 2:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 2 \n";
								break;
							case 3:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": passenger 3 \n";
								break;
							default:
								lg << "Anims: Found 'f_an" << mode << " ERROR: submode not found \n";
								submode = -1;
								break;
							}
							break;
						case 3:
							switch (submode)
							{
							case 0:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed \n";
								break;
							case 1:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed and f_spoiler \n";
								xdata.spoilerFrames.push_back(frame);
								break;
							default:
								lg << "Anims: Found 'f_an" << mode << " ERROR: submode not found \n";
								submode = -1;
								break;
							}
							break;
						case 4:
							switch (submode)
							{
							case 0:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": brake \n";
								break;
							case 1:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed brake \n";
								break;
							case 2:
								lg << "Anims: Found 'f_an" << mode << "' " << submode << ": high speed brake and f_spoiler \n";
								xdata.spoilerFrames.push_back(frame);
								break;
							default:
								lg << "Anims: Found 'f_an" << mode << " ERROR: submode not found \n";
								submode = -1;
								break;
							}
							break;
						default:
							lg << "Anims: Found 'f_an' ERROR: mode not found \n";
							mode = -1;
							break;
						}

						if (mode >= 0 && submode >= 0)
						{
							if (CreateMatrixBackup(frame))
							{
								F_an *an = new F_an(frame);
								an->mode = mode;
								an->submode = submode;

								xdata.anims.push_back(an);
								FRAME_EXTENSION(frame)->owner = vehicle;
							}
						}
					}
				}

				//if (vehicle->m_nVehicleSubClass == VEHICLE_QUAD) // Quadbike on SetModelEvent is 0, idkw
				//{
					// tricycle fork
					found = name.find("f_trifork");
					if (found != string::npos)
					{
						lg << "Trifork: Found 'f_trifork' \n";
						if (CreateMatrixBackup(frame))
						{
							xdata.triforkFrame = frame;
							FRAME_EXTENSION(frame)->owner = vehicle;

							//if (!frame->child)
							//{
							RwFrame *wheelFrame = CClumpModelInfo::GetFrameFromId(vehicle->m_pRwClump, 5);
							RwFrameAddChild(frame->child, wheelFrame->child);
							//}
						}
					}
				//}

				if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX || vehicle->m_nVehicleSubClass == VEHICLE_QUAD)
				{
					// footpeg driver
					found = name.find("f_fpeg1");
					if (found != string::npos) 
					{
						lg << "Footpegs: Found 'f_fpeg1' (footpeg driver) \n";
						if (CreateMatrixBackup(frame)) {
							xdata.fpegFront.push_back(new F_footpegs(frame));
							FRAME_EXTENSION(frame)->owner = vehicle;
						}
					}

					// footpeg passenger
					found = name.find("f_fpeg2");
					if (found != string::npos) 
					{
						lg << "Footpegs: Found 'f_fpeg2' (footpeg passenger) \n";
						if (CreateMatrixBackup(frame)) {
							xdata.fpegBack.push_back(new F_footpegs(frame));
							FRAME_EXTENSION(frame)->owner = vehicle;
						}
					}
				}

				// Hitch
				found = name.find("f_hitch"); 
				if (found != string::npos) 
				{
					lg << "FunctionalHitch: Found 'f_hitch' \n";
					xdata.hitchFrame = frame;
					FRAME_EXTENSION(frame)->owner = vehicle;
				}

				// Cop light
				found = name.find("f_coplight");
				if (found != string::npos) 
				{
					lg << "CopLight: Found 'f_coplight' \n";
					xdata.coplightFrame = frame;
					FRAME_EXTENSION(frame)->owner = vehicle;
					vehicle->m_vehicleAudio.m_bModelWithSiren = true;
					if (RwFrame *child = frame->child) 
					{
						const string childname = GetFrameNodeName(child);
						found = childname.find("outpoint");
						if (found != string::npos) 
						{
							lg << "CopLight: Found 'outpoint' \n";
							xdata.coplightoutFrame = child;
						}
					}
				}

				// Taxi light
				found = name.find("f_taxilight");
				if (found != string::npos)
				{
					lg << "TaxiLight: Found 'f_taxilight' \n";
					xdata.taxilightFrame = frame;
					FRAME_EXTENSION(frame)->owner = vehicle;
				}

				// Wheel
				if (!bReSearch) {
					found = name.find("f_wheel");
					if (found != string::npos) 
					{
						lg << "Wheel: Found 'f_wheel' at " << name << " \n";
						for (int i = 0; i < 6; i++) 
						{
							if (!xdata.wheelFrame[i]) 
							{
								xdata.wheelFrame[i] = frame;
								FRAME_EXTENSION(frame)->owner = vehicle;
								break;
							}
						}
					}
				}

				// Popup lights
				found = name.find("f_pop");
				if (found != string::npos)
				{
					if (CreateMatrixBackup(frame)) {
						if (name[found + 5] == 'l') {
							lg << "Popup lights: Found 'f_popl' \n";
							xdata.popupFrame[0] = frame;
						}
						else
						{
							lg << "Popup lights: Found 'f_popr' \n";
							xdata.popupFrame[1] = frame;
						}
						FRAME_EXTENSION(frame)->owner = vehicle;
					}
				}

				// Spoiler
				found = name.find("f_spoiler");
				if (found != string::npos)
				{
					lg << "Spoiler: Found 'f_spoiler' \n";
					xdata.spoilerFrames.push_back(frame);
					FRAME_EXTENSION(frame)->owner = vehicle;
				}

				// Steer
				found = name.find("f_steer");
				if (found != string::npos)
				{
					lg << "Steer: Found 'f_steer' \n";
					if (CreateMatrixBackup(frame)) {
						xdata.steer.push_back(frame);
						FRAME_EXTENSION(frame)->owner = vehicle;
					}
				}

			} // end of "f_"
			else // retrocompatibility
			{
				if (!IVFinstalled) {
					ExtendedData &xdata = xData.Get(vehicle);
					found = name.find("movsteer");
					if (found != string::npos)
					{
						found = name.find("movsteer");
						if (found != string::npos)
						{
							if (name[8] == '_') {
								if (isdigit(name[9])) {
									lg << "Retrocompatibility: Found 'movsteer_*' \n";
									if (CreateMatrixBackup(frame)) {
										xdata.steer.push_back(frame);
										FRAME_EXTENSION(frame)->owner = vehicle;
									}
								}
							}
							else {
								lg << "Retrocompatibility: Found 'movsteer' \n";
								if (CreateMatrixBackup(frame)) {
									xdata.steer.push_back(frame);
									FRAME_EXTENSION(frame)->owner = vehicle;
								}
							}
						}
					}
				}
				if (!APPinstalled) {
					ExtendedData &xdata = xData.Get(vehicle);
					found = name.find("steering_dummy");
					if (found != string::npos)
					{
						if (frame->child) {
							lg << "Retrocompatibility: Found 'steering' \n";
							if (CreateMatrixBackup(frame->child)) {
								xdata.steer.push_back(frame->child);
								FRAME_EXTENSION(frame->child)->owner = vehicle;
							}
						}
					}
				}
			}

			// Characteristics
			FindVehicleCharacteristicsFromNode(frame, vehicle, bReSearch);

			/////////////////////////////////////////
			if (RwFrame * newFrame = frame->child)  FindNodesRecursive(newFrame, vehicle, bReSearch, bOnExtras);
			if (RwFrame * newFrame = frame->next)   FindNodesRecursive(newFrame, vehicle, bReSearch, bOnExtras);
			return;
		}
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

} vehfuncs;


RwFrame *__cdecl CustomRwFrameForAllChildren_AddUpgrade(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data)
{
	if (RwFrame * newFrame = frame->child)  CustomRwFrameForAllChildren_AddUpgrade_Recurse(newFrame, callback, data);
	return frame;
}

RwFrame *__cdecl CustomRwFrameForAllChildren_AddUpgrade_Recurse(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data)
{
	FRAME_EXTENSION(frame)->flags.bNeverRender = true;
	FRAME_EXTENSION(frame)->flags.bDontDestroyOnRemoveUpgrade = true;

	if (RwFrame * newFrame = frame->child)  CustomRwFrameForAllChildren_AddUpgrade_Recurse(newFrame, callback, data);
	if (RwFrame * newFrame = frame->next)   CustomRwFrameForAllChildren_AddUpgrade_Recurse(newFrame, callback, data);
	return frame;
}

RwFrame *__cdecl CustomRwFrameForAllChildren_RemoveUpgrade(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data)
{
	if (frame) CustomRwFrameForAllChildren_RemoveUpgrade_Recurse(frame, callback, data);
	return frame;
}

RwFrame *__cdecl CustomRwFrameForAllChildren_RemoveUpgrade_Recurse(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data)
{
	if (FRAME_EXTENSION(frame)->flags.bDontDestroyOnRemoveUpgrade) {
		FRAME_EXTENSION(frame)->flags.bNeverRender = false;
	}
	else {
		RemoveObjectsCB(frame, data);
	}

	if (RwFrame * newFrame = frame->child)  CustomRwFrameForAllChildren_RemoveUpgrade_Recurse(newFrame, callback, data);
	if (RwFrame * newFrame = frame->next)   CustomRwFrameForAllChildren_RemoveUpgrade_Recurse(newFrame, callback, data);
	return frame;
}




RwFrame *__cdecl CustomRwFrameForAllObjects_Upgrades(RwFrame *frame, RpAtomicCallBack callback, void *data)
{
	if (!rwLinkListEmpty(&frame->objectList))
	{
		RwObjectHasFrame * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		while (current != end) {
			atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);

			if (!callback((RpAtomic *)atomic, data)) break;

			current = rwLLLinkGetNext(current);
		}
	}
	return frame;
}

RwFrame *__cdecl CustomCollapseFramesCB(RwFrame *frame, void *data)
{
	RwFrameForAllChildren(frame, (RwFrameCallBack)CustomCollapseFramesCB, data);
	RwFrameForAllObjects(frame, (RwObjectCallBack)CustomMoveObjectsCB, data);

	/*const string parentName = GetFrameNodeName(RwFrameGetParent(frame));
	found = parentName.find("wheel_");
	if (found != string::npos)
	{
		found = parentName.find("_dummy");
		{
			RwFrameDestroy(frame);
		}
	}*/

	return frame;
}

RpAtomic *__cdecl CustomMoveObjectsCB(RpAtomic *atomic, RwFrame *frame)
{
	RwFrame * frameAtomic = GetObjectParent((RwObject*)atomic);
	RwFrame * frameAtomicParent = RwFrameGetParent(frameAtomic);

	int frameId = CVisibilityPlugins::GetFrameHierarchyId(frameAtomicParent);

	// If parent is dummy, always collapse
	if (frameId > 0)
	{
		const string frameAtomicName = GetFrameNodeName(frameAtomic);
		if (frameAtomicName[0] == 'e')
		{
			if (frameAtomicName[1] == 'x' && frameAtomicName[2] == 't' && frameAtomicName[3] == 'r' && frameAtomicName[4] == 'a') 
			{
				return atomic;
			}
		}
		else
		{
			if (frameAtomicName[0] == 'f' && frameAtomicName[1] == '_')
			{
				return atomic;
			}
		}
		RpAtomicSetFrame(atomic, frame);
		return atomic;
	}


	// If parent isn't dummy, collapse if is alpha and damageable
	if ((uint32_t)atomic->renderCallBack == AtomicAlphaCallBack) {
		RwFrame * parent = RwFrameGetParent(frameAtomic);

		while (parent != frame) {
			const string parentName = GetFrameNodeName(parent);
			if (parentName[0] == 'f') 
			{
				if (parentName[1] == '_') 
				{
					return atomic;
				}
			}
			parent = RwFrameGetParent(parent);
			if (!parent) break;
		} 

		const string frameAtomicName = GetFrameNodeName(frameAtomic);
		int len = frameAtomicName.length();

		if (len >= 4) 
		{
			if (frameAtomicName[0] == 'e')
			{
				if (frameAtomicName[1] == 'x' && frameAtomicName[2] == 't' && frameAtomicName[3] == 'r' && frameAtomicName[4] == 'a')
				{
					return atomic;
				}
			}
			else 
			{
				if (frameAtomicName[0] == 'f' && frameAtomicName[1] == '_')
				{
					return atomic;
				}
			}

			if (frameAtomicName[(len - 3)] == '_'
				&& frameAtomicName[(len - 2)] == 'o'
				&& frameAtomicName[(len - 1)] == 'k') 
			{
				RpAtomicSetFrame(atomic, frame);
				return atomic;
			}

			if (frameAtomicName[(len - 4)] == '_'
				&& frameAtomicName[(len - 3)] == 'd'
				&& frameAtomicName[(len - 2)] == 'a'
				&& frameAtomicName[(len - 1)] == 'm') 
			{
				RpAtomicSetFrame(atomic, frame);
				return atomic;
			}
		}
	}

	return atomic;
}
