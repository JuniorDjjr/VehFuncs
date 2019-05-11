#include "VehFuncsCommon.h"
#include "FixMaterials.h"

void FixMaterials(RpClump * clump, bool fixIVF) 
{
	RpClumpForAllAtomics(reinterpret_cast<RpClump*>(clump), AtomicCallback, (bool *)fixIVF);
	return;
}

RpMaterial *MaterialCallback(RpMaterial *material, void * fixIVF) 
{
	if (CheckMaterialColors(material)) ResetMaterialColors(material);
	return material;
}

RpAtomic *AtomicCallback(RpAtomic *atomic, void * fixIVF) 
{
	if (atomic->geometry) 
	{
		atomic->geometry->flags |= rpGEOMETRYMODULATEMATERIALCOLOR;
		if (fixIVF) RpGeometryForAllMaterials(atomic->geometry, MaterialCallback, (bool *)fixIVF);
	}

	return atomic;
}

bool CheckMaterialColors(RpMaterial * material) 
{
	if (material->color.red == 255) 
	{
		if (material->color.blue == 0) 
		{
			if (material->color.green >= 173 && material->color.green <= 175) return true;
			if (material->color.green >= 56 && material->color.green <= 60) return true;
		}
		return false;
	}
	if (material->color.green == 255) 
	{
		if (material->color.blue == 0) 
		{
			if (material->color.red >= 181 && material->color.red <= 185) return true;
		}
		if (material->color.red == 0) 
		{
			if (material->color.blue >= 198 && material->color.blue <= 200) return true;
		}
		return false;
	}
	if (material->color.blue == 255) 
	{
		if (material->color.red == 0) 
		{
			if (material->color.green >= 16 && material->color.green <= 18) return true;
		}
		return false;
	}
	return false;
}

void ResetMaterialColors(RpMaterial * material) 
{
	material->color.red = 0xFF; material->color.green = 0xFF; material->color.blue = 0xFF;
	return;
}

