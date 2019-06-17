#pragma once
#include "plugin.h"

RwFrame * HideAllNodesRecursive(RwFrame *frame);
void HideAllAtomics(RwFrame * frame);
void SetFrameFirstAtomicVisibility(RwFrame * frame, bool visible);
void SetFrameFirstAtomicVisibilityButFixedDamage(RwFrame *frame, bool visible, CVehicle *vehicle);
void HideAllAtomicsExcept(RwFrame * frame, int except);
bool IsFrameFirstAtomicVisible(RwFrame * frame);
void SetFrameAtomicsRenderCallback(RwFrame * frame, RpAtomicCallBackRender rendercallback);
