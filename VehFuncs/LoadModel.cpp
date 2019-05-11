#include "plugin.h"
#include "CStreaming.h"

bool LoadModel(int model)
{
	unsigned char oldFlags = CStreaming::ms_aInfoForModel[model].m_nFlags;
	CStreaming::RequestModel(model, GAME_REQUIRED);
	CStreaming::LoadAllRequestedModels(false);
	if (CStreaming::ms_aInfoForModel[model].m_nLoadState == LOADSTATE_LOADED)
	{
		if (!(oldFlags & GAME_REQUIRED))
		{
			CStreaming::SetModelIsDeletable(model);
			CStreaming::SetModelTxdIsDeletable(model);
		}
		return true;
	}
	return false;
}
