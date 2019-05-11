﻿/*
    Author: Junior_Djjr - MixMods.com.br
    Created using https://github.com/DK22Pac/plugin-sdk
*/
#include "VehFuncs.h"

// Mod utilities & API
#include "AtomicsVisibility.h"
#include "IndieVehHandlingsAPI.h"
#include "CustomSeed.h"
#include "Matrixbackup.h"
  
// Mod funcs
#include "Patches.h"
#include "FixMaterials.h"
#include "DigitalSpeedo.h"
#include "Characteristics.h"
#include "RecursiveExtras.h"
#include "Hitch.h"
#include "GearAndFan.h"
#include "Shake.h"
#include "Pedal.h"
#include "Footpegs.h"
 
// Dependences
#include "../injector/assembly.hpp"
#include "CVisibilityPlugins.h"
#include "CTxdStore.h"
#include "NodeName.h"
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
uint32_t AtomicAlphaCallBack;
VehicleExtendedData<ExtendedData> remInfo;
fstream lg;


///////////////////////////////////////////////////////////////////////////////////////////////////


class VehFuncs
{
public:
	VehFuncs()
	{
		// -- On plugin init
		lg.open("VehFuncs.log", fstream::out | fstream::trunc);
		static bool reInit = false;
		remInfo = getremInfo();

		// Fix remaps on txd files named with digits.
		Patches::FixRemapTxdName();


		// -- On game init
		Events::initGameEvent += []
		{
			lg << "Core: Init\n";

			srand(time(0));
			StoreHandlingData();
			ApplyGSX();
			
			AtomicAlphaCallBack = ReadMemory<int>(0x4C7842, false);


			// -- Patches
			lg << "Core: Applying Patches\n";

			// Preprocess hierarchy don't remove frames
			MakeJMP(0x004C8E30, CustomCollapseFramesCB);

			// Preprocess hierarchy find damage atomics to apply damageable
			MakeCALL(0x4C9173, Patches::FindDamage::CustomFindDamageAtomics, true);
			WriteMemory<uint32_t>(0x4C916D + 1, memory_pointer(Patches::FindDamage::CustomFindDamageAtomicsCB).as_int(), true);

			// Render bus driver
			MakeNOP(0x0064BCB3, 6);
			MakeCALL(0x0064BCB3, Patches::RenderBus);

			// Cop functions
			MakeJMP(0x006D2374, Patches::IsLawEnforcement);
			Patches::PatchForCoplights();

			// Hitch patch
			int *vmt = (int*)0x00871120;
			int *getlink = vmt + (0xF0 / sizeof(vmt));
			Patches::Hitch::setOriginalFun((Patches::Hitch::GetTowBarPos_t)ReadMemory<int*>(getlink, true));
			WriteMemory(getlink, memory_pointer(Patches::Hitch::GetTowBarPosToHook).as_int(), true);

			lg << "Core: Started\n";
		};


		// -- On plugins attach
		Events::attachRwPluginsEvent += []() 
		{
			FramePluginOffset = RwFrameRegisterPlugin(sizeof(FramePlugin), PLUGIN_ID_STR, (RwPluginObjectConstructor)FramePlugin::Init, (RwPluginObjectDestructor)FramePlugin::Destroy, (RwPluginObjectCopy)FramePlugin::Copy);
		};


		// -- On game process
		Events::gameProcessEvent += []() 
		{
			static bool IVFunhooked = false;
			if (!IVFunhooked) 
			{
				WriteMemory(0x004C9148, (int)0x004C8E30, true); // unhook IVF collapse frames
				lg << "Core: Unhook\n";
				IVFunhooked = true;
			}
		};


		// -- On vehicle set model
		Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelId) 
		{
			if (vehicle->m_nVehicleSubClass != VEHICLE_HELI) // SilentPatch incompatibility with plugin-sdk...
			{
				ExtendedData &xdata = remInfo.Get(vehicle);

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
							lg << "Custom Seed: Seed " << customSeed.seed << " was set to " << customSeed.pvehicle << "\n";
							xdata.randomSeed = customSeed.seed;
							customSeedList.remove(*it);

							if (customSeedList.size() <= 0) break;
						}
					}
				}

				// Clear temp class list
				list<string> &classList = getClassList();
				classList.clear();

				// Process all nodes
				RwFrame * rootFrame = GetObjectParent(&vehicle->m_pRwClump->object);
				FindNodesRecursive(rootFrame, vehicle, false, false);

				// Post set
				//SetCharacteristics(vehicle, false); //nothing yet
				xdata.nodesProcessInRender = true;
				xdata.nodesProcessed = true;
			}
		};


		// On vehicle render
        Events::vehicleRenderEvent += [](CVehicle *vehicle)
		{
			if (vehicle->m_nVehicleSubClass != VEHICLE_HELI) // SilentPatch incompatibility with plugin-sdk...
			{
				ExtendedData &xdata = remInfo.Get(vehicle);
				tHandlingData * handling;
				bool bReSearch = false;

				if (IsIndieHandling(vehicle, &handling)) 
				{
					bReSearch = ExtraInfoBitReSearch(vehicle, handling);

					// for IndieVehHandlings
					if (!xdata.nodesProcessedSecFrame || bReSearch) 
					{
						SetCharacteristicsForIndieHandling(vehicle, bReSearch);
						xdata.nodesProcessedSecFrame = true;
					}
				}

				// Process
				if (bReSearch)
				{
					// Reset extended data
					xdata.ReInitForReSearch();

					// Clear temp class list
					list<string> &classList = getClassList();
					classList.clear();

					// Process all nodes
					xdata.randomSeedUsage = 0;
					RwFrame * rootFrame = GetObjectParent(&vehicle->m_pRwClump->object);
					FindNodesRecursive(rootFrame, vehicle, bReSearch, false);

					// Post set
					//SetCharacteristics(vehicle, bReSearch);
					SetCharacteristicsInRender(vehicle, bReSearch);
					xdata.nodesProcessInRender = true;
				}
				else
				{
					// Post set
					if (xdata.nodesProcessInRender)
					{
						// Fix materials
						FixMaterials(vehicle->m_pRwClump, true);

						// Post set
						SetCharacteristicsInRender(vehicle, bReSearch);

						// Set wheels
						if (xdata.wheelFrame[0]) SetWheel(xdata.wheelFrame, vehicle);

						xdata.nodesProcessInRender = false;

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

				}


				///////////////////////////////////////////////////////////////////////////////////////

				// Process smooth pedal
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

				// Process speedo
				if (xdata.speedoFrame != nullptr) 
				{
					if (xdata.speedoDigits != nullptr) 
					{
						ProcessDigitalSpeedo(vehicle, xdata.speedoFrame);
					}
				}

				///////////////////////////////////////////////////////////////////////////////////////

				if (vehicle->m_nFlags.bEngineOn && !vehicle->m_nFlags.bEngineBroken)
				{
					// Process gear
					if (!xdata.gearFrame.empty()) { ProcessRotatePart(vehicle, xdata.gearFrame, true); }

					// Process fan
					if (!xdata.fanFrame.empty()) { ProcessRotatePart(vehicle, xdata.fanFrame, false); }
				}
				// Process shake
				if (!xdata.shakeFrame.empty()) { ProcessShake(vehicle, xdata.shakeFrame); }

				// Process gas pedal
				if (!xdata.gaspedalFrame.empty()) { ProcessPedal(vehicle, xdata.gaspedalFrame, 1); }

				// Process brake pedal
				if (!xdata.brakepedalFrame.empty()) { ProcessPedal(vehicle, xdata.brakepedalFrame, 2); }

				// Process footpegs
				if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX)
				{
					if (!xdata.fpeg1Frame.empty()) { ProcessFootpegs(vehicle, xdata.fpeg1Frame, 1); }
					if (!xdata.fpeg2Frame.empty()) { ProcessFootpegs(vehicle, xdata.fpeg2Frame, 2); }
				}

				// Process body tilt
				//if (xdata.bodyTilt > 0.0f) { ProcessBodyTilt(vehicle); }


				///////////////////////////////////////////////////////////////////////////////////////
			}
        };


		// Flush log during unfocus (ie minimizing)
		Events::onPauseAllSounds += []
		{
			lg.flush();
		};


		// On game reload
		Events::reInitGameEvent += [] // It's also called first time the game is initialized
		{
			if (reInit)
			{
				lg << "Core: Reinitializing game...\n";
				// ...?
			}
			else reInit = true;
		};
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
				ExtendedData &xdata = remInfo.Get(vehicle);
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

								lg << "Extras: --- Starting process --- \n";
								ProcessExtraRecursive(frame, vehicle);
								lg << "Extras: --- Ending process --- \n";
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
				}

				// Fan
				found = name.find("f_fan");
				if (found != string::npos) 
				{
					lg << "Gear: Found 'f_fan' \n";
					xdata.fanFrame.push_back(frame);
				}

				// Shake
				found = name.find("f_shake");
				if (found != string::npos) 
				{
					lg << "Shake: Found 'f_shake' \n";
					xdata.shakeFrame.push_back(frame);

					FRAME_EXTENSION(frame)->origMatrix = CreateMatrixBackup(&frame->modelling);
				}

				// Gaspedal
				found = name.find("f_gas");
				if (found != string::npos) 
				{
					lg << "Pedal: Found 'f_gas' \n";
					xdata.gaspedalFrame.push_back(frame);

					FRAME_EXTENSION(frame)->origMatrix = CreateMatrixBackup(&frame->modelling);
				}

				// brakepedal
				found = name.find("f_brake");
				if (found != string::npos) 
				{
					lg << "Pedal: Found 'f_brake' \n";
					xdata.brakepedalFrame.push_back(frame);

					FRAME_EXTENSION(frame)->origMatrix = CreateMatrixBackup(&frame->modelling);
				}

				if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX) {

					// footpeg driver
					found = name.find("f_fpeg1");
					if (found != string::npos) 
					{
						lg << "Footpegs: Found 'f_fpeg1' (footpeg driver) \n";
						xdata.fpeg1Frame.push_back(frame);

						FRAME_EXTENSION(frame)->origMatrix = CreateMatrixBackup(&frame->modelling);
					}

					// footpeg passenger
					found = name.find("f_fpeg2");
					if (found != string::npos) 
					{
						lg << "Footpegs: Found 'f_fpeg2' (footpeg passenger) \n";
						xdata.fpeg2Frame.push_back(frame);

						FRAME_EXTENSION(frame)->origMatrix = CreateMatrixBackup(&frame->modelling);
					}
				}

				// Hitch
				found = name.find("f_hitch");
				if (found != string::npos) 
				{
					lg << "FunctionalHitch: Found 'f_hitch' \n";
					xdata.hitchFrame = frame;
				}

				// Cop light
				found = name.find("f_coplight");
				if (found != string::npos) 
				{
					lg << "CopLight: Found 'f_coplight' \n";
					xdata.coplightFrame = frame;
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
								break;
							}
						}
					}
				}
			}// end of "f_"

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


RwFrame *__cdecl CustomCollapseFramesCB(RwFrame *frame, void *data)
{
	RwFrameForAllChildren(frame, (RwFrameCallBack)CustomCollapseFramesCB, data);
	RwFrameForAllObjects(frame, (RwObjectCallBack)CustomMoveObjectsCB, data);

	/*
	size_t found;

	const string parentName = GetFrameNodeName(RwFrameGetParent(frame));
	found = parentName.find("wheel_");
	if (found != string::npos)
	{
		found = parentName.find("_dummy");
		{
			RwFrameDestroy(frame);
		}
	}
	*/
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