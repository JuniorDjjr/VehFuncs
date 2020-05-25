#include "VehFuncsCommon.h"
#include "../injector/assembly.hpp"

using namespace injector;

// Damageable rear wings (based on DK22Pac code)
void MyCustomVehicleDamageComponent(CAutomobile *car, int component, float intensity) {
	if (component == 11 || component == 12) {
		RwFrame *frame = nullptr;
		unsigned char panelId;
		if (component == 11) {
			panelId = 2;
			frame = CClumpModelInfo::GetFrameFromName(car->m_pRwClump, "wing_lr_dummy");
		}
		else {
			panelId = 3;
			frame = CClumpModelInfo::GetFrameFromName(car->m_pRwClump, "wing_rr_dummy");
		}
		if (frame) { // if component found
			if (car->m_damageManager.ProgressPanelDamage(panelId)) {
				unsigned int state = car->m_damageManager.GetPanelStatus(panelId);
				switch (state) {
				case DAMSTATE_DAMAGED:
				case DAMSTATE_OPENED:
					car->SetComponentVisibility(frame, 2); // switch to _dam object
					break;
				case DAMSTATE_OPENED_DAMAGED:
					RwFrame *savedComp = car->m_aCarNodes[0];
					car->m_aCarNodes[0] = frame;
					car->SpawnFlyingComponent(0, 5); // spawn detached object
					car->m_aCarNodes[0] = savedComp;
					car->SetComponentVisibility(frame, 0); // hide component
					break;
				}
			}
		}
	}
}

void __declspec(naked) DamageableRearWingsHook() {
	__asm {
		movzx   eax, [esp + 0x78]
		fld[esp + 0x74]
		push    ecx
		fstp[esp]  // intensity
		push    eax    // component
		push    esi    // vehicle
		call    MyCustomVehicleDamageComponent
		add     esp, 0xC
		movzx   eax, [esp + 0x78]
		mov     edx, 0x6A7BAE
		jmp     edx
	}
}

void PatchDamageableRearWings() {

	plugin::patch::RedirectJump(0x6A7BA9, DamageableRearWingsHook);

	//WriteMemory<float>(0x8D3158, 0.0f, false); // any impact
	MakeNOP(0x6A7ED8, 6);
	MakeNOP(0x6A7EE1, 9);

	MakeInline<0x006A7F68, 0x006A7F68 + 5>([](reg_pack& regs)
	{
		int i = (int)regs.ebx;

		CVehicle *vehicle = (CVehicle *)regs.esi;

		// Default return
		*(uintptr_t*)(regs.esp - 0x4) = 0x6A8044;

		// Damage also rear wings to cars (by default the code only damages rear doors for vans)
		if (i & 0x20)
		{
			if (vehicle->m_nVehicleFlags.bIsVan)
			{
				*(uintptr_t*)(regs.esp - 0x4) = 0x6A7F6D;
			}
			else {
				MyCustomVehicleDamageComponent(reinterpret_cast<CAutomobile*>(vehicle), 11, 10.0f);
			}
		}
		else {
			if (i && i & 0x40) {
				if (vehicle->m_nVehicleFlags.bIsVan)
				{
					*(uintptr_t*)(regs.esp - 0x4) = 0x6A7FA3;
				}
				else {
					MyCustomVehicleDamageComponent(reinterpret_cast<CAutomobile*>(vehicle), 12, 10.0f);
				}
			}
		}
	});
}