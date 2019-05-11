#include "plugin.h"
#include "../injector/assembly.hpp"

namespace Patches {
	extern uint32_t defaultCopLightSwitch;
	void FixRemapTxdName();
	void PatchForCoplights();
	void __declspec() UseCopLights();
	void __declspec() RenderBus();
	void __declspec() IsLawEnforcement();
	void __cdecl RenderBusCheck(CVehicle *veh);
	void __cdecl IsLawEnforcementCheck(CVehicle *veh);
	namespace Hitch {
		typedef bool(__thiscall *GetTowBarPos_t)(CAutomobile *ths, RwV3d *point, char a3, CAutomobile *a4);
		extern void setOriginalFun(GetTowBarPos_t f);
		void __declspec() GetTowBarPosToHook();
	}
	namespace FindDamage {
		RwFrame *__cdecl CustomFindDamageAtomics(RwFrame *frame, RwFrame *(__cdecl *callback)(RwFrame *, void *), void *data);
		RwFrame *__cdecl CustomFindDamageAtomicsCB(RwFrame *frame, void *data);
	}
}