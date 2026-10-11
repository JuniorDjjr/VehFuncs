#include "VehFuncsCommon.h"
#include "../injector/assembly.hpp"

using namespace injector;

// Damageable rear wings (based on DK22Pac code)
void DamageRearWing(CAutomobile *car, int component) {
	RwFrame *frame = nullptr;
	unsigned char panelId = (component == 11) ? 2 : 3;
	if (component == 11) {
		frame = CClumpModelInfo::GetFrameFromName(car->m_pRwClump, (char*)"wing_lr_dummy");
	}
	else {
		frame = CClumpModelInfo::GetFrameFromName(car->m_pRwClump, (char*)"wing_rr_dummy");
	}
	if (frame) {
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

void __cdecl ProcessRearLightsDamage(CAutomobile* car) {
	if (!car || !car->m_matrix) return;

	// Front headlight damage calculation from GTA SA (CDamageManager::ApplyDamage):
	// Total damage = fDamageIntensity * 4.0f * m_fCollisionDamageMultiplier * 2.8f
	// Minimum threshold is 150.0f
	float mult = car->m_pHandlingData ? car->m_pHandlingData->m_fCollisionDamageMultiplier : 1.0f;
	float totalDamage = car->m_fDamageIntensity * 4.0f * mult * 2.8f;
	if (totalDamage < 150.0f) {
		return;
	}

	unsigned int component = car->m_nPieceType;
	if (component == 3 || component == 5 || component == 19) return; // Front bumper, bonnet, windscreen

	CVector delta = car->m_vecLastCollisionPosn - car->m_matrix->pos;
	float localY = delta.x * car->m_matrix->up.x + delta.y * car->m_matrix->up.y + delta.z * car->m_matrix->up.z;

	// Only process rear impacts (behind vehicle center or rear bumper/boot)
	if (localY >= 0.0f && component != 4 && component != 6) {
		return;
	}

	float localX = delta.x * car->m_matrix->right.x + delta.y * car->m_matrix->right.y + delta.z * car->m_matrix->right.z;

	bool damageLeftWing = false;
	bool damageRightWing = false;

	if (localX > 0.2f) {
		car->m_damageManager.SetLightStatus(LIGHT_REAR_RIGHT, 1);
		if (totalDamage >= 280.0f || localX > 0.6f) damageRightWing = true;
	}
	else if (localX < -0.2f) {
		car->m_damageManager.SetLightStatus(LIGHT_REAR_LEFT, 1);
		if (totalDamage >= 280.0f || localX < -0.6f) damageLeftWing = true;
	}
	else {
		// Center impact
		if (totalDamage >= 280.0f) {
			car->m_damageManager.SetLightStatus(LIGHT_REAR_LEFT, 1);
			car->m_damageManager.SetLightStatus(LIGHT_REAR_RIGHT, 1);
			damageLeftWing = true;
			damageRightWing = true;
		}
		else {
			car->m_damageManager.SetLightStatus(localX >= 0.0f ? LIGHT_REAR_RIGHT : LIGHT_REAR_LEFT, 1);
		}
	}

	if (damageLeftWing)  DamageRearWing(car, 11);
	if (damageRightWing) DamageRearWing(car, 12);
}

void __declspec(naked) VehicleDamageRearLightsHook() {
	__asm {
		pushad
		pushfd

		push    esi // car (CAutomobile*)
		call    ProcessRearLightsDamage
		add     esp, 4

		popfd
		popad

		// Original 10 bytes replaced at 0x6A8044:
		// 0x6A8044: fld dword ptr [esp + 0x74] (4 bytes)
		// 0x6A8048: mov edx, dword ptr [esi + 0x384] (6 bytes)
		fld     dword ptr [esp + 0x74]
		mov     edx, dword ptr [esi + 0x384]

		push    0x6A804E
		ret
	}
}

void PatchDamageableRearWings() {
	// Hook in CAutomobile::VehicleDamage right where component damage finishes and before light sound is checked
	MakeJMP(0x6A8044, VehicleDamageRearLightsHook);
	MakeNOP(0x6A8044 + 5, 5);
}