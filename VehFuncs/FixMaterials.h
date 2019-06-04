#pragma once
#include "plugin.h"


void FixMaterials(RpClump * clump);
RpMaterial *MaterialCallback(RpMaterial *material, void *data);
RpAtomic *AtomicCallback(RpAtomic *atomic, void *data);
bool CheckIVFMaterialColors(RpMaterial * material);
