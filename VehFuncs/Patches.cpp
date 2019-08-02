#include "plugin.h"
#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CModelInfo.h"
#include "Patches.h"
#include "../injector/assembly.hpp"

namespace Patches
{
	int valid = 0;

	void __cdecl RenderBusCheck(CVehicle *veh)
	{
		valid = 0;
		if (veh->m_nFlags.bIsBus) 
		{
			int model = veh->m_nModelIndex;
			if (model == MODEL_BUS)
			{
				valid = 1;
			}
			else
			{
				ExtendedData &xdata = remInfo.Get(veh);
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
		valid = !FRAME_EXTENSION(frame)->flags.bNeverRender;
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

	void __cdecl IsLawEnforcementCheck(CVehicle *veh)
	{
		valid = false;
		if (veh && veh->m_nModelIndex > 0) {
			ExtendedData &xdata = remInfo.Get(veh);
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
			add     eax, -427
			push    6D2379h
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
		if (veh->m_nFlags.bSirenOrAlarm) 
		{
			ExtendedData &xdata = remInfo.Get(veh);
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


	void PatchForCoplights()
	{
		// No cops
		Patches::defaultCopLightSwitch = ReadMemory<uint32_t>(0x006ACCCC, true);
		WriteMemory<uint32_t>(0x006ACCCC, (uint32_t)UseCopLights, true);

		// Normal cops
		MakeInline<0x006AB5BE, 0x006AB5BE+8>([](reg_pack& regs)
		{
			CVehicle *veh = (CVehicle*)regs.esi;
			ExtendedData &xdata = remInfo.Get(veh);

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

				RwV3d * framePos = &xdata.coplightFrame->modelling.pos;

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
		/*MakeNOP(0x006ABBC1, 5);
		MakeInline<0x006ABBC1>([](reg_pack& regs)
		{
			CVehicle *veh = (CVehicle*)regs.esi;

			ExtendedData &xdata = remInfo.Get(veh);

			RwV3d *in = (RwV3d*)(regs.esp + 0x2C);

			if (xdata.coplightFrame) 
			{
				RwV3d * framePos = &xdata.coplightFrame->modelling.pos;
				in->x = framePos->x;
				in->y = framePos->y;
				in->z = framePos->z;
			}
			else 
			{
				in->x = 0.0;
				in->y = 1.2;
				in->z = 0.5;
			}

			//regs.eax = *(uint8_t*)(regs.esi + 0x42D); // mov     al, [esi+42Dh]
			regs.eax = 0xB7CB84;  // mov     eax, _ZN6CTimer22m_snTimeInMillisecondsE
			//ProcessCoplightFBIRancher(regs);
		});*/
	}

	void FixRemapTxdName()
	{
		MakeInline<0x004C9360>([](reg_pack& regs)
		{
			const char *txdName = *(const char **)(regs.esp + 0x4);
			uint16_t txdId = *(uint16_t*)(regs.esp + 0x8);

			size_t len = strlen(txdName);
			if (isdigit(txdName[len - 1]))
			{
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

				uint32_t minVehModel = ReadMemory<uint32_t>(0x4C93CB + 1);
				uint32_t maxVehModel = ReadMemory<uint32_t>(0x4C93C2 + 1);

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
		});
		WriteMemory<uint8_t>(0x4C9365, 0xC3, true); //retn
	}

	// -- Hitch (by Fabio)
	namespace Hitch
	{
		auto GetTowBarPosOriginal = GetTowBarPos_t(nullptr);

		static CAutomobile *veh = nullptr;
		static RwV3d *p = nullptr;
		static char a0 = 0;
		static CAutomobile *a1 = nullptr;
		static int result = 0;

		static bool isCustomCar(CAutomobile *mob, RwV3d &point)
		{
			ExtendedData &xdata = remInfo.Get(mob);
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

			return GetTowBarPosOriginal(thisA, point, a3, a4);
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
				//lg << "DONE" << "\n";
				return frame;
			}
			//lg << "PROCESSING" << "\n";
			RwFrameForAllChildren(frame, CustomFindDamageAtomicsCB, data);
			return frame;
		}
	}
}
