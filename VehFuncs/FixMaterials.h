#pragma once
#include "plugin.h"

void FixMaterials(RpClump * clump, bool fixIVF);
RpMaterial *MaterialCallback(RpMaterial *material, void * fixIVF);
RpAtomic *AtomicCallback(RpAtomic *atomic, void * fixIVF);
void ResetMaterialColors(RpMaterial * material);
bool CheckMaterialColors(RpMaterial * material);