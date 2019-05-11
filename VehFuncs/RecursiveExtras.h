#pragma once
#include "plugin.h"

void ProcessClassesRecursive(RwFrame * frame, CVehicle * vehicle, bool bReSearch);
void ProcessExtraRecursive(RwFrame * frame, CVehicle * vehicle);
void RemoveFrameClassFromNormalArray(RwFrame * frameClass, RwFrame * frameArray[]);
bool FrameIsOtherClass(RwFrame * frame);
bool IsDamAtomic(RpAtomic *a);
bool IsOkAtomic(RpAtomic *a);

