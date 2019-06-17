#include "VehFuncsCommon.h"
#include "NodeName.h"
#include "CTimer.h"
#include "CGeneral.h"

void ProcessTrifork(CVehicle *vehicle, RwFrame* frame)
{
	if (frame->object.parent)
	{
		RwFrame *wheelFrame = CClumpModelInfo::GetFrameFromId(vehicle->m_pRwClump, 5);
		RwFrame *wheelFinalFrame = frame->child->child;
		if (wheelFrame && wheelFinalFrame)
		{
			/*
			frame->modelling.at.x = wheelFrame->modelling.at.x;
			frame->modelling.at.y = wheelFrame->modelling.at.y;
			frame->modelling.right.x = wheelFrame->modelling.right.x;
			frame->modelling.right.y = wheelFrame->modelling.right.y;
			frame->modelling.up.x = wheelFrame->modelling.up.x;
			frame->modelling.up.y = wheelFrame->modelling.up.y;
			*/

			float angle = CGeneral::GetATanOfXY(wheelFrame->modelling.right.x, wheelFrame->modelling.right.y) * 57.295776f - 180.0f;
			while (angle < 0.0) angle += 360.0;

			RestoreMatrixBackup(&frame->modelling, FRAME_EXTENSION(frame)->origMatrix);
			RwFrameRotate(frame, (RwV3d *)0x008D2E18, angle, rwCOMBINEPRECONCAT);

			CAutomobile *autom = (CAutomobile *)vehicle;

			//RwV3d vec = RwV3d{ 0.0f, 0.0f, diffGuardWheel };
			//RwFrameTranslate(wheelFinalFrame, &vec, rwCOMBINEREPLACE);

			RwFrameRotate(wheelFinalFrame, (RwV3d *)0x008D2E00, (autom->m_fWheelRotation[0] * 57.295776f), rwCOMBINEREPLACE);

			RwFrameUpdateObjects(wheelFinalFrame);
			RwFrameUpdateObjects(frame);
		}
	}
	else
	{
		ExtendedData &xdata = remInfo.Get(vehicle);
		xdata.triforkFrame = nullptr;
	}
}
