#include "plugin.h"
#include "CStreaming.h"

bool LoadModel(int model)
{
	//unsigned char oldFlags = CStreaming::ms_aInfoForModel[model].m_nFlags;
	if (CStreaming::ms_aInfoForModel[model].m_nCdSize > 0)
	{
		CStreaming::RequestModel(model, (GAME_REQUIRED | PRIORITY_REQUEST));
		CStreaming::LoadAllRequestedModels(true);
		if (CStreaming::ms_aInfoForModel[model].m_nLoadState == LOADSTATE_LOADED)
		{
			/*if (!(oldFlags & GAME_REQUIRED))
			{
				CStreaming::SetModelIsDeletable(model);
				CStreaming::SetModelTxdIsDeletable(model);
			}*/
			return true;
		}
	}
	return false;
}

void MarkModelAsNoLongerNeeded(int model)
{
	CStreaming::SetModelIsDeletable(model);
	CStreaming::SetModelTxdIsDeletable(model);
}
