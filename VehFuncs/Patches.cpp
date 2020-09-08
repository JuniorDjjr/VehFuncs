#include "plugin.h"
#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CModelInfo.h"
#include "Patches.h"
#include "CVisibilityPlugins.h"
#include "CCamera.h"
#include "CTxdStore.h"
#include "CStreaming.h"
#include <math.h>
#include "../injector/assembly.hpp"

float ftest = 1.0f;

extern bool iniLogNoTextureFound;
RwTexDictionary *vehicletxdArray[4];
int vehicletxdIndexArray[4];
bool anyAdditionalVehicleTxd;

extern void LogCrashText(string str);
extern void LogVehicleModelWithText(string str1, int vehicleModel, string str2);
extern int lastRenderedVehicleModel;
extern int lastInitializedVehicleModel;
extern CVehicle* lastInitializedVehicle;

namespace Patches
{
	int valid = 0;

	void __cdecl RenderBusCheck(CVehicle *veh)
	{
		valid = 1;
		if (veh->m_nVehicleFlags.bIsBus) 
		{
			int model = veh->m_nModelIndex;
			if (model == MODEL_BUS)
			{
				valid = 1;
			}
			else
			{
				ExtendedData &xdata = xData.Get(veh);
				valid = xdata.flags.bBusRender ? 1 : 0;
			}
		}
	}

	void __declspec(naked) RenderBus()
	{
		__asm {
			pushad
			pushfd

			push eax // CVehicle
			call RenderBusCheck
			add esp, 4 // params

			popfd
			popad

			cmp valid, 1
			ret
		}
	}

	void __cdecl NeverRenderCheck(RpAtomic *atomic)
	{
		RwFrame *frame = (RwFrame *)atomic->object.object.parent;
		if (FRAME_EXTENSION(frame)->flags.bNeverRender) {
			valid = false;
			return;
		}
		if (atomic->clump->object.type == 2 && FRAME_EXTENSION(frame)->LODdist != -100) {
			int LODdistLevel = FRAME_EXTENSION(frame)->LODdist;
			float LODdist = FRAME_EXTENSION(frame)->LODdist * 3.0f * TheCamera.m_fLODDistMultiplier;
			//if (useLog) lg << pow(LODdist, 3) << " " << *(float*)0x00C88024 << "\n";
			if (LODdistLevel > 0) // >
			{
				if (LODdistLevel == 9)
				{
					if (*(float*)0x00C88024 < *(float*)0x00C88040) // only with vlo
					{
						valid = false;
						return;
					}
				}
				else
				{
					if (*(float*)0x00C88024 < pow(LODdist, 3)) {
						valid = false;
						return;
					}
				}
			}
			else
			{
				if (LODdistLevel < 0) // < 
				{
					if (LODdistLevel == -9)
					{
						if (*(float*)0x00C88024 > *(float*)0x00C88040) // only without vlo
						{
							valid = false;
							return;
						}
					}
					else
					{
						LODdist *= -1.0f;
						if (*(float*)0x00C88024 > pow(LODdist, 3)) {
							valid = false;
							return;
						}
					}
				}
			}
		}
		valid = true;
		return;
	}

	void __declspec(naked) NeverRender()
	{
		__asm {

			push edi // RpAtomic
			call NeverRenderCheck
			add esp, 4 // params

			mov     eax, [edi+4]
			push    eax
			mov     edx, 7F0990h
			call    edx

			cmp     valid, 1
			je      NeverRender_IsValid
			add esp, 4
			push    749B4Eh
			ret
				
			NeverRender_IsValid:
			push    749B47h
			ret
		}
	}

	void __cdecl ForceRenderCustomLODCheck(RpAtomic *atomic)
	{
		RwFrame *frame = (RwFrame *)atomic->object.object.parent;
		ftest = *(float*)0x00C88024; //gVehicleDistanceFromCamera
		if (atomic->clump->object.type == 2 && FRAME_EXTENSION(frame)->LODdist != -100) {
			if (FRAME_EXTENSION(frame)->LODdist >= 0) // >
			{
				if (*(float*)0x00C88024 < *(float*)0x00C8803C) // consider vehicle draw distance too (lod1)
				{
					ftest = 0.0f; // the camera is near, render it
				}
			}
		}
	}

	void __declspec(naked) ForceRenderCustomLOD()
	{
		__asm {
			push    esi
			mov     esi, [esp + 0Ch]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 733247h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODAlpha()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 733F86h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODBoatAlpha()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 7344A6h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODBoat()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 733556h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODBig()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 733426h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODBigAlpha()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 734376h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODTrain()
	{
		__asm {
			push    esi
			mov     esi, [esp + 0Ch]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 733337h
			ret
		}
	}

	void __declspec(naked) ForceRenderCustomLODTrainAlpha()
	{
		__asm {
			push    esi
			mov     esi, [esp + 8h]
			push    esi
			call ForceRenderCustomLODCheck
			add esp, 4
			pop esi

			fld ftest
			push 734246h
			ret
		}
	}

	RwFrame* MyGetWheelPosnGetFrameById(RpClump *clump, int id)
	{
		RwFrame *frame = CClumpModelInfo::GetFrameFromId(clump, id);
		if (!frame)
		{
			switch (id)
			{
			case eCarNodes::CAR_WHEEL_LF:
				frame = CClumpModelInfo::GetFrameFromName(clump, "wheel_lf_dummy");
				break;
			case eCarNodes::CAR_WHEEL_LB:
				frame = CClumpModelInfo::GetFrameFromName(clump, "wheel_lb_dummy");
				break;
			case eCarNodes::CAR_WHEEL_RF:
				frame = CClumpModelInfo::GetFrameFromName(clump, "wheel_rf_dummy");
				break;
			case eCarNodes::CAR_WHEEL_RB:
				frame = CClumpModelInfo::GetFrameFromName(clump, "wheel_rb_dummy");
				break;
			default:
				frame = CClumpModelInfo::GetFrameFromName(clump, "wheel_lf_dummy");
				break;
			}
			if (frame)
			{
				if (useLog)
				{
					lg << "ERROR GetWheelPosn on vehicle model ID " << lastInitializedVehicleModel << " wheel node id " << id << " was going to crash. Fixed. \n";
					lg.flush();
				}
			}
			else
			{
				string logText = "GAME CRASH GetWheelPosn on vehicle model ID ";
				logText.append(to_string(lastInitializedVehicleModel));
				logText.append(": Required wheel node id ");
				logText.append(to_string(id));
				logText.append(" doesn't exist. Check '0x004C7DAD' on MixMods' Crash List.\n");
				LogCrashText(logText);
			}
		}
		return frame;
	}

	void __cdecl IsLawEnforcementCheck(CVehicle *veh)
	{
		valid = false;
		if (veh && veh->m_nModelIndex > 0) {
			ExtendedData &xdata = xData.Get(veh);
			if (&xdata != nullptr) {
				if (xdata.coplightFrame != nullptr) valid = true;
			}
		}
	}

	void __declspec(naked) IsLawEnforcement()
	{
		__asm {
			pushad
			pushfd

			push ecx // CVehicle
			call IsLawEnforcementCheck
			add esp, 4 // params

			popfd
			popad

			cmp     valid, 1
			je      IsLawEnforcement_IsValid
			cmp     eax, 0AEh

			push    6D237Eh
			ret

			IsLawEnforcement_IsValid:
			mov     al, 1
		    retn
		}
	}


	uint32_t defaultCopLightSwitch = 0x00000000;

	void __cdecl UseCopLightsCheck(CVehicle *veh)
	{
		valid = 0;
		if (veh->m_nVehicleFlags.bSirenOrAlarm) 
		{
			ExtendedData &xdata = xData.Get(veh);
			valid = xdata.coplightFrame ? 1 : 0;
		}
	}

	void __declspec(naked) UseCopLights()
	{
		__asm {
			pushad
			pushfd

			push esi // CVehicle
			call UseCopLightsCheck
			add esp, 4 // params

			popfd
			popad

			cmp     valid, 1
			je      UseCopLights_Valid
			push    defaultCopLightSwitch
			ret

			UseCopLights_Valid:
			push    6AB3A6h
			ret
		}
	}

	void CheckCrashWorldRemove_ShowCrash_Func()
	{
		LogCrashText("GAME CRASH CWorld::Remove: Entity is null. Check '0x00563289' on MixMods' Crash List.");
	}

	void CheckCrashWorldRemove_ShowCrash_FuncB(CEntity *entity)
	{
		LogVehicleModelWithText("GAME CRASH CWorld::Remove: On model ID ", entity->m_nModelIndex, ": Maybe is NOT related to this model, but other game bug. Check '0x00563289' on MixMods' Crash List.");
	}

	void __declspec(naked) CheckCrashWorldRemove()
	{
		__asm {
			mov     esi, [esp + 8]

			test esi, esi
			jz CheckCrashWorldRemove_ShowCrash

			mov     eax, [esi + 0]

			test eax, eax
			jz CheckCrashWorldRemove_ShowCrashB

			CheckCrashWorldRemove_Finalize:
			push 563287h
			ret

			CheckCrashWorldRemove_ShowCrash:
			pushad
			pushfd
			call CheckCrashWorldRemove_ShowCrash_Func
			popfd
			popad
			jmp CheckCrashWorldRemove_Finalize

			CheckCrashWorldRemove_ShowCrashB:
			pushad
			pushfd
			push esi
			call CheckCrashWorldRemove_ShowCrash_FuncB
			add esp, 4
			popfd
			popad
			jmp CheckCrashWorldRemove_Finalize
		}
	}

	void CheckCrashMatrixOperator_ShowCrash_Func()
	{
		if (lastRenderedVehicleModel <= 0)
		{
			LogCrashText("GAME CRASH Matrix operator*. Maybe is NOT caused by vehicle. Check '0x0059BE3E' on MixMods' Crash List.");
		}
		else
		{
			LogVehicleModelWithText("GAME CRASH Matrix operator*. The rendering vehicle model ID is ", lastRenderedVehicleModel, ", maybe the problem is with this vehicle model. Check '0x0059BE3E' on MixMods' Crash List.");
		}
	}

	void __declspec(naked) CheckCrashMatrixOperator()
	{
		__asm {
			test eax, eax
			jz CheckCrashMatrixOperator_ShowCrash
			test ecx, ecx
			jz CheckCrashMatrixOperator_ShowCrash

			CheckCrashMatrixOperator_Finalize:
			fld     dword ptr[eax + 10h]
			fmul    dword ptr[ecx + 4h]
			push    59BE41h
			ret

			CheckCrashMatrixOperator_ShowCrash:
			pushad
			pushfd
			call CheckCrashMatrixOperator_ShowCrash_Func
			popfd
			popad
			jmp CheckCrashMatrixOperator_Finalize
		}
	}

	int __cdecl CheckCrashFillFrameArrayCB(RwFrame* frame)
	{
		int id = CVisibilityPlugins::GetFrameHierarchyId(frame);
		// REF: http://www.mixmods.com.br/2020/06/mais-um-problema-grave-comum-mods.html
		if (id >= 25) // 25 is the limit for CAutomobile, not for CBike etc. But you simply can't name the node with caracters with ASCII less than 31 (32 is space, 65 is A etc), so this is simple and safe.
		{
			// Consider generic node and don't write it to nodes array.
			CVisibilityPlugins::SetFrameHierarchyId(frame, 0);
			//LogVehicleModelWithText("GAME CRASH FillFrameArrayCB on vehicle model ID ", lastInitializedVehicleModel, ": Some problem with the model, nodes etc. Check '0x004C53A6' on MixMods' Crash List. VehFuncs will try to avoid this crash.");
			if (useLog) lg << "ERROR: Vehicle model ID " << lastInitializedVehicleModel << " was going to cause a crash on " << GetFrameNodeName(frame) << ". Check '0x004C53A6' on MixMods' Crash List. Manually fixed. \n";
			return 0;
		}
		return id;
	}

	void PatchForCoplights()
	{
		// Taxi
		MakeInline<0x006ABACE, 0x006ABACE + 8>([](reg_pack& regs)
		{
			CVehicle *veh = (CVehicle*)regs.esi;
			ExtendedData &xdata = xData.Get(veh);

			RwV3d *point = (RwV3d*)(regs.esp + 0x2C);

			if (xdata.taxilightFrame)
			{
				RwV3d *framePos = &xdata.taxilightFrame->modelling.pos;

				point->x = framePos->x;
				point->y = framePos->y;
				point->z = framePos->z;
			}
			else {
				point->x = 0.0f; // original code
			}
		});

		// No cops
		Patches::defaultCopLightSwitch = ReadMemory<uint32_t>(0x006ACCCC, true);
		WriteMemory<uint32_t>(0x006ACCCC, (uint32_t)UseCopLights, true);

		// Normal cops
		MakeInline<0x006AB5BE, 0x006AB5BE+8>([](reg_pack& regs)
		{
			CVehicle *veh = (CVehicle*)regs.esi;
			ExtendedData &xdata = xData.Get(veh);

			if (xdata.coplightFrame) 
			{
				RwV3d *position = (RwV3d*)(regs.esp + 0x20);
				RwV3d *point = (RwV3d*)(regs.esp + 0x54);
				RwV3d *outpoint = (RwV3d*)(regs.esp + 0x44);

				const string name = GetFrameNodeName(xdata.coplightFrame);
				if (name.length() >= 16) 
				{
					//f_coplightFFFFFF
					if (name[10] != '_') 
					{
						int rgb = stoi(&name[10], 0, 16);
						if (rgb != 0x000000) 
						{
							regs.eax = rgb / 0x10000;            //R
							regs.edx = (rgb / 0x100) % 0x100;    //G
							regs.ecx = rgb % 0x100;              //B
						}
					}
				}

				RwV3d *framePos = &xdata.coplightFrame->modelling.pos;

				point->x = framePos->x;
				point->y = framePos->y;
				point->z = framePos->z;

				position->x = 0.0;
				position->y = framePos->y;
				position->z = framePos->z;

				if (xdata.coplightoutFrame) 
				{
					RwV3d * outframePos = &xdata.coplightoutFrame->modelling.pos;

					outpoint->x = outframePos->x;
					outpoint->y = outframePos->y;
					outpoint->z = outframePos->z;
				}
				else 
				{
					outpoint->x = (framePos->x * -1.0f);
					outpoint->y = framePos->y;
					outpoint->z = framePos->z;
				}
			}

			*(uint8_t*)(regs.esp + 0x50) = (uint8_t)regs.eax; // mov     byte ptr [esp+194h+var_144], al
			*(uint8_t*)(regs.esp + 0x3C) = (uint8_t)regs.ecx; // mov     byte ptr [esp+194h+var_158], cl
		});

		// FBI Rancher
		MakeInline<0x006ABBE1, 0x006ABBE1 + 6>([](reg_pack& regs)
		{
			CVehicle *veh = (CVehicle*)regs.esi;

			ExtendedData &xdata = xData.Get(veh);

			RwV3d *point = (RwV3d*)(regs.esp + 0x2C);

			if (xdata.coplightFrame) 
			{
				RwV3d * framePos = &xdata.coplightFrame->modelling.pos;
				point->x = framePos->x;
				point->y = framePos->y;
				point->z = framePos->z;

				const string name = GetFrameNodeName(xdata.coplightFrame);
				if (name[10] == 'f')
				{
					*(uintptr_t*)(regs.esp - 0x4) = 0x6ABC1E;
				}
			}
			else 
			{
				// original code
				point->x = 0.0f;
				point->y = 1.2f;
				point->z = 0.5f;
			}
		});
		WriteMemory<uint8_t>(0x6ABBC9 + 0, 0x0F, true);
		WriteMemory<uint8_t>(0x6ABBC9 + 1, 0x84, true);
		WriteMemory<uint8_t>(0x6ABBC9 + 2, 0xA2, true);
		WriteMemory<uint8_t>(0x6ABBC9 + 3, 0x00, true);
		WriteMemory<uint8_t>(0x6ABBC9 + 4, 0x00, true);
		WriteMemory<uint8_t>(0x6ABBC9 + 5, 0x00, true);
		MakeNOP(0x6ABBC9 + 6, 18, true);
	}

	// Fix remaps on txd files named with digits.
	void CustomAssignRemapTxd(const char *txdName, uint16_t txdId)
	{
		if (txdName) {

			size_t len = strlen(txdName);

			if (len > 1) {

				if (isdigit(txdName[len - 1]))
				{
					// ignore 'radar' txds, just a little performance improvement, because 'GetModelInfo' isn't so fast
					if (len >= 7 && txdName[0] == 'r' && txdName[1] == 'a' && txdName[2] == 'd' && txdName[3] == 'a' && txdName[4] == 'r')
					{
					}
					else
					{
						if (txdName[0] == 'v' && strncmp(txdName, "vehicle", 7) == 0)
						{
							int arrayIndex = txdName[len - 1] - '0' - 2; // first is 2
							if (arrayIndex < 4) {
								vehicletxdIndexArray[arrayIndex] = txdId;
								CTxdStore::AddRef(vehicletxdIndexArray[arrayIndex]);
								anyAdditionalVehicleTxd = true;
								if (useLog) lg << "Found additional vehicle.txd \n";
							}
							else {
								if (useLog) lg << "ERROR: vehicle*.txd limit is only up to 'vehicle5.txd' \n";
								if (useLog) lg.flush();
							}
						}

						do
						{
							len--;
							if (txdName[len] == '_')
							{
								len--;
								break;
							}
						} while (isdigit(txdName[len]));

						len++;

						char modelTxdName[32];
						strncpy(modelTxdName, txdName, (len));
						modelTxdName[len] = 0;

						CBaseModelInfo *modelInfo;

						uint32_t minVehModel = ReadMemory<uint32_t>(0x4C93CB + 1, true);
						uint32_t maxVehModel = ReadMemory<uint32_t>(0x4C93C2 + 1, true);

						if (maxVehModel == 630) { //default
							modelInfo = CModelInfo::GetModelInfo(modelTxdName, minVehModel, maxVehModel);
						}
						else { //patched
							modelInfo = CModelInfo::GetModelInfo(modelTxdName, NULL); // Paintjobs working on any ID
						}
						if (modelInfo)
						{
							if (modelInfo->GetModelType() == 6)
							{
								CVehicleModelInfo *vehModelInfo = (CVehicleModelInfo *)modelInfo;
								vehModelInfo->AddRemap(txdId);
							}
						}
					}
				}
			}
		}
	}

	// -- Hitch (by Fabio)
	namespace Hitch
	{
		auto GetTowBarPosOriginal = GetTowBarPos_t(nullptr);
		auto GetTowBarPosOriginal_Trailer = GetTowBarPos_t(nullptr);

		static CAutomobile *veh = nullptr;
		static RwV3d *p = nullptr;
		static char a0 = 0;
		static CAutomobile *a1 = nullptr;
		static int result = 0;

		static bool isCustomCar(CAutomobile *mob, RwV3d &point)
		{
			ExtendedData &xdata = xData.Get(mob);
			if (xdata.hitchFrame == nullptr) return false;

			RpAtomic * atomic = (RpAtomic*)GetFirstObject(xdata.hitchFrame);
			if (atomic) if (atomic->object.object.flags & 0x4) {}
			else return false; // If atomic is visible

			RwMatrix * matrix = RwFrameGetLTM(xdata.hitchFrame);

			point.x = matrix->pos.x;
			point.y = matrix->pos.y;
			point.z = matrix->pos.z;
			return true;
		}

		static bool GetTowBarPos(CAutomobile *thisA, RwV3d *point, char a3, CAutomobile *a4)
		{
			RwV3d p;

			if (isCustomCar(thisA, p))
			{
				*point = p;
				return true;
			}

			if (thisA->m_nVehicleSubClass != VEHICLE_TRAILER)
				return GetTowBarPosOriginal(thisA, point, a3, a4);
			else
				return GetTowBarPosOriginal_Trailer(thisA, point, a3, a4);
		}

		static void wrapper()
		{
			result = GetTowBarPos(veh, p, a0, a1);
		}

		void __declspec(naked) /*__fastcall*/ GetTowBarPosToHook(/*CAutomobile *thisA, int EDX, RwV3D *point, char a3, int a4*/)
		{
			/*veh = thisA;
			p = point;
			a0 = a3;
			a1 = a4;
			*/
			__asm
			{
				mov veh, ecx
				mov eax, [esp + 4]
				mov p, eax

				mov al, [esp + 8]
				mov a0, al

				mov eax, [esp + 12]
				mov a1, eax

				pushad
				pushfd

				call wrapper

				popfd
				popad

				mov eax, result

				retn 0xC
			}
		}

		extern void setOriginalFun(GetTowBarPos_t f)
		{
			GetTowBarPosOriginal = f;
		}
		extern void setOriginalFun_Trailer(GetTowBarPos_t f)
		{
			GetTowBarPosOriginal_Trailer = f;
		}
	}

	namespace FindDamage {
		RwFrame *__cdecl CustomFindDamageAtomics(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data)
		{
			RwFrame *child; // eax
			RwFrame *next; // esi

			if (!callback(frame, data))
				return frame;

			child = frame->child;
			if (child)
			{
				do
				{
					next = child->next;
					if (!callback(child, data))
						break;
					child = next;
				} while (next);
			}
			return frame;
		}

		RwFrame *__cdecl CustomFindDamageAtomicsCB(RwFrame *frame, void *data)
		{
			RwFrameForAllObjects(frame, (RwObjectCallBack)0x4C7BD0, data);
			if (/*ok*/*(uint32_t*)data > 0 && /*dam*/*(uint32_t*)(&data + 4) > 0)  // for performance
			{ 
				//if (useLog) lg << "DONE \n";
				return frame;
			}
			//if (useLog) lg << "PROCESSING \n";
			RwFrameForAllChildren(frame, CustomFindDamageAtomicsCB, data);
			return frame;
		}
	}


	RwTexture *__cdecl Custom_RwTexDictionaryFindNamedTexture(RwTexDictionary *dict, const char *name)
	{
		RwTexture *texture;

		texture = RwTexDictionaryFindNamedTexture(dict, name);
		if (texture) return texture;

		if (anyAdditionalVehicleTxd) {
			for (int i = 0; i < 4; ++i)
			{
				if (vehicletxdArray[i])
				{
					texture = RwTexDictionaryFindNamedTexture(vehicletxdArray[i], name);
					if (texture) return texture;
				}
			}
		}
		if (iniLogNoTextureFound && useLog) lg << "Can't find texture " << name << "\n";
		return nullptr;
	}

	void LoadAdditionalVehicleTxd()
	{
		if (anyAdditionalVehicleTxd) {
			for (int i = 0; i < 4; ++i)
			{
				if (vehicletxdIndexArray[i]) {
					CStreaming::RequestTxdModel(vehicletxdIndexArray[i], (eStreamingFlags::PRIORITY_REQUEST | eStreamingFlags::KEEP_IN_MEMORY));
					//CStreaming::RequestTxdModel(vehicletxdIndexArray[i], 8);
				}
			}

			CStreaming::LoadAllRequestedModels(true);

			for (int i = 0; i < 4; ++i)
			{
				if (vehicletxdIndexArray[i]) {
					vehicletxdArray[i] = ((RwTexDictionary *(__cdecl*)(int)) 0x408340)(vehicletxdIndexArray[i]); //size_t __cdecl getTexDictionary(int txdIndex)
				}
			}
		}
	}
}
