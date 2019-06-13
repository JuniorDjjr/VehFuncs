#include "VehFuncsCommon.h"
#include "AtomicsVisibility.h"
#include "CVisibilityPlugins.h"

RwFrame * HideAllNodesRecursive(RwFrame *frame) 
{
	SetFrameFirstAtomicVisibility(frame, false);

	if (RwFrame * nextFrame = frame->child) HideAllNodesRecursive(nextFrame);
	if (RwFrame * nextFrame = frame->next)  HideAllNodesRecursive(nextFrame);
	return frame;
}

void HideAllAtomics(RwFrame * frame) 
{
	if (!rwLinkListEmpty(&frame->objectList)) 
	{
		RwObjectHasFrame * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		while (current != end) {
			atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
			atomic->object.flags &= ~0x4; // clear

			current = rwLLLinkGetNext(current);
		}
	}
	return;
}

void ShowAllAtomics(RwFrame * frame)
{
	if (!rwLinkListEmpty(&frame->objectList))
	{
		RwObjectHasFrame * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		while (current != end) {
			atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
			atomic->object.flags &= ~0x4; // clear

			current = rwLLLinkGetNext(current);
		}
	}
	return;
}

void SetFrameAtomicsRenderCallback(RwFrame * frame, RpAtomicCallBackRender rendercallback)
{
	if (!rwLinkListEmpty(&frame->objectList))
	{
		RpAtomic * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		while (current != end) {
			atomic = (RpAtomic*)rwLLLinkGetData(current, RwObjectHasFrame, lFrame);

			CVisibilityPlugins::SetAtomicRenderCallback(atomic, rendercallback);

			current = rwLLLinkGetNext(current);
		}
	}
	return;
}

bool IsFrameFirstAtomicVisible(RwFrame * frame)
{
	RpAtomic * atomic = (RpAtomic*)GetFirstObject(frame);
	if (atomic) if (atomic->object.object.flags & 0x4) return true;
	return false;
}

void SetFrameFirstAtomicVisibility(RwFrame * frame, bool visible) 
{
	RpAtomic * atomic = (RpAtomic*)GetFirstObject(frame);
	if (visible) atomic->object.object.flags |= 0x4; // set
	else atomic->object.object.flags &= ~0x4; // clear
	return;
}

void HideAllAtomicsExcept(RwFrame * frame, int except)
{
	if (!rwLinkListEmpty(&frame->objectList))
	{
		RwObjectHasFrame * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		int i = 0;
		while (current != end)
		{
			atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
			if (i == except) 
			{
				atomic->object.flags |= 0x4; // set
			}
			else 
			{
				atomic->object.flags &= ~0x4; // clear
			}

			current = rwLLLinkGetNext(current);
			i++;
		}
	}
	return;
}
