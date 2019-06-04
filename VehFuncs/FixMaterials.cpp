#include "VehFuncsCommon.h"
#include "FixMaterials.h"

extern bool fixIVF;
extern CVehicle *curVehicle;

enum MatFuncType {
	nothing,
	ivf,
	taxi
};
MatFuncType CheckMaterials(RpMaterial * material);

void FixMaterials(RpClump * clump) 
{
	uint32_t data = 0;
	RpClumpForAllAtomics(reinterpret_cast<RpClump*>(clump), AtomicCallback, (void *)data);
	return;
}

RpMaterial *MaterialCallback(RpMaterial *material, void *data)
{
	ExtendedData &xdata = remInfo.Get(curVehicle);

	switch (CheckMaterials(material))
	{
	case ivf:
		material->color.red = 0xFF; material->color.green = 0xFF; material->color.blue = 0xFF;
		break;
	case taxi:
		xdata.taxiSignMaterial = material;
		lg << "Taxi: Found sign material \n";
		break;
	default:
		break;
	}

	return material;
}

RpAtomic *AtomicCallback(RpAtomic *atomic, void *data) 
{
	if (atomic->geometry) 
	{
		atomic->geometry->flags |= rpGEOMETRYMODULATEMATERIALCOLOR;
		RpGeometryForAllMaterials(atomic->geometry, MaterialCallback, data);
	}
	return atomic;
}

MatFuncType CheckMaterials(RpMaterial * material)
{
	// Fix Improved Vehicle Features material colors
	if (fixIVF)
	{
		if (material->color.red == 255)
		{
			if (material->color.blue == 0)
			{
				if (material->color.green >= 173 && material->color.green <= 175) return ivf;
				if (material->color.green >= 56 && material->color.green <= 60) return ivf;
			}
		}
		else if (material->color.green == 255)
		{
			if (material->color.blue == 0)
			{
				if (material->color.red >= 181 && material->color.red <= 185) return ivf;
			}
			if (material->color.red == 0)
			{
				if (material->color.blue >= 198 && material->color.blue <= 200) return ivf;
			}
		}
		else if (material->color.blue == 255)
		{
			if (material->color.red == 0)
			{
				if (material->color.green >= 16 && material->color.green <= 18) return ivf;
			}
		}
	}

	if (material->texture)
	{
		string texName = material->texture->name;

		if (texName.length() > 5)
		{
			size_t found;

			// Taxi sign
			found = texName.find("92sign64");
			if (found != string::npos)
			{
				return taxi;
			}
		}
	}

	return nothing;
}
