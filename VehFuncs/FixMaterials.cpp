#include "VehFuncsCommon.h"
#include "FixMaterials.h"

extern bool IVFinstalled;
extern CVehicle *curVehicle;

enum MatFuncType {
	nothing,
	ivf,
	taxi
};
MatFuncType CheckMaterials(RpMaterial * material, RpAtomic * atomic);

void FixMaterials(RpClump * clump) 
{
	uint32_t data = 0;
	RpClumpForAllAtomics(reinterpret_cast<RpClump*>(clump), AtomicCallback, (void *)data);
	return;
}

RpMaterial *MaterialCallback(RpMaterial *material, void *data)
{
	ExtendedData &xdata = remInfo.Get(curVehicle);

	switch (CheckMaterials(material, (RpAtomic *)data))
	{
	case MatFuncType::ivf:
		/*if (material->texture)
		{
			string name = material->texture->name;
			if (name[15] == '8')
			{
				if (name.length() == 16)
				{
					size_t found = name.find("vehiclelights128");
					if (found != string::npos)
					{
						lg << "Not valid IVF material \n";
						break;
					}
				}
			}
			else
			{
				if (name[15] == 'n')
				{
					if (name.length() == 18)
					{
						size_t found = name.find("vehiclelightson128");
						if (found != string::npos)
						{
							lg << "Not valid on IVF material \n";
							break;
						}
					}
				}
			}
			break;
		}*/
		lg << "Found IVF material \n";
		material->color.red = 0xFF; material->color.green = 0xFF; material->color.blue = 0xFF;
		break;
	case MatFuncType::taxi:
		xdata.taxiSignMaterial = material;
		//lg << "Taxi: Found sign material \n";
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
		RpGeometryForAllMaterials(atomic->geometry, MaterialCallback, atomic);
	}
	return atomic;
}

MatFuncType CheckMaterials(RpMaterial * material, RpAtomic *atomic)
{
	if (material->color.red != 255 || material->color.green != 255 || material->color.blue != 255)
	{
		// Fix Improved Vehicle Features material colors
		// We are not fixing emergency lights here, at least for now, because need more conditions and that case is not important (like, the color is red)
		if (!IVFinstalled)
		{
			if (material->color.red == 255)
			{
				if (material->color.blue == 0)
				{
					if (material->color.green >= 173 && material->color.green <= 175) return MatFuncType::ivf;
					if (material->color.green >= 56 && material->color.green <= 60) return MatFuncType::ivf;
				}
			}
			else if (material->color.green == 255)
			{
				if (material->color.blue == 0)
				{
					if (material->color.red >= 181 && material->color.red <= 185) return MatFuncType::ivf;
				}
				if (material->color.red == 0)
				{
					if (material->color.blue >= 198 && material->color.blue <= 200) return MatFuncType::ivf;
				}
			}
			else if (material->color.blue == 255)
			{
				if (material->color.red == 0)
				{
					if (material->color.green >= 16 && material->color.green <= 18) return MatFuncType::ivf;
				}
			}
		}
		atomic->geometry->flags |= rpGEOMETRYMODULATEMATERIALCOLOR;
	}

	if (material->texture)
	{
		string texName = material->texture->name;

		if (texName.length() > 7)
		{
			size_t found;

			// Taxi sign
			found = texName.find("92sign64");
			if (found != string::npos)
			{
				return MatFuncType::taxi;
			}
		}
	}

	return MatFuncType::nothing;
}
