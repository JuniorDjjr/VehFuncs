/*#include "plugin.h"
#include "CBike.h"
#include "CPed.h"
#include "CBaseModelInfo.h"
#include "CVehicleModelInfo.h"
#include "CModelInfo.h"
#include "CMatrix.h"
#include "RenderWare.h"
#include "ePedBones.h"

void HandsFix(CBike *thisBike, CPed *ped)
{
	CBike *bike; // ebx
	char result; // al
	RwFrame *v4; // eax
	RpHAnimHierarchy *v5; // eax
	__int16 bikeID; // cx
	RpHAnimHierarchy *clump; // esi
	CVector *dummyBarPos; // eax
	signed int boneIndex; // edi
	RwMatrix *boneMatrixArray; // eax
	CVector *v11; // eax
	RwV3d *v12; // eax
	float v13; // eax
	RwV3d *v14; // eax
	float v15; // ecx
	float v16; // edx
	float v17; // eax
	signed int v18; // edi
	RwV3d *v19; // eax
	float v20; // ST70_4
	float v21; // ST6C_4
	float v22; // ST54_4
	double v23; // st7
	double v24; // st6
	float v25; // ST60_4
	float v26; // ST64_4
	float v27; // ST68_4
	signed int v28; // edi
	RwV3d *v29; // eax
	double v30; // st7
	double v31; // st6
	float v32; // ST60_4
	float v33; // ST64_4
	float v34; // ST68_4
	signed int v35; // edi
	RwV3d *v36; // eax
	double v37; // st7
	double v38; // st6
	float v39; // ST60_4
	float v40; // ST64_4
	float v41; // ST68_4
	signed int v42; // edi
	RwV3d *v43; // eax
	float v44; // ecx
	float v45; // edx
	float v46; // ST64_4
	double v47; // st6
	float v48; // ST60_4
	float v49; // ST64_4
	float v50; // ST68_4
	signed int v51; // edi
	double v52; // st7
	RwV3d *v53; // eax
	float v54; // ST70_4
	double v55; // st6
	float v56; // ST60_4
	float v57; // edx
	float v58; // ST64_4
	double v59; // st6
	double v60; // st6
	float v61; // ST54_4
	float v62; // ST58_4
	float v63; // ST5C_4
	signed int v64; // edi
	RwV3d *v65; // eax
	float v66; // ST54_4
	float v67; // ST58_4
	double v68; // st7
	float v69; // ST54_4
	float v70; // ST58_4
	float v71; // ST5C_4
	RwV3d *v72; // eax
	float v73; // ecx
	float v74; // edx
	signed int v75; // edi
	RwV3d *v76; // eax
	float v77; // ST6C_4
	float v78; // ST70_4
	float v79; // ST60_4
	double v80; // st7
	double v81; // st6
	float v82; // ST54_4
	float v83; // ST58_4
	float v84; // ST5C_4
	signed int v85; // edi
	RwV3d *v86; // eax
	double v87; // st7
	double v88; // st6
	float v89; // ST54_4
	float v90; // ST58_4
	float v91; // ST5C_4
	signed int v92; // edi
	RwV3d *v93; // eax
	double v94; // st7
	double v95; // st6
	float v96; // ST54_4
	float v97; // ST58_4
	float v98; // ST5C_4
	signed int v99; // edi
	RwV3d *v100; // eax
	float v101; // ecx
	float v102; // edx
	float v103; // ST58_4
	double v104; // st6
	float v105; // ST54_4
	float v106; // ST58_4
	float v107; // ST5C_4
	signed int v108; // edi
	double v109; // st7
	RwV3d *v110; // eax
	double v111; // st6
	float v112; // ST54_4
	float v113; // ecx
	float v114; // edx
	double v115; // st6
	double v116; // st6
	float v117; // ST54_4
	float v118; // ST58_4
	float v119; // ST5C_4
	signed int v120; // edi
	RwV3d *v121; // eax
	float v122; // ST54_4
	float v123; // ST58_4
	double v124; // st7
	float v125; // ST54_4
	float v126; // ST58_4
	float v127; // ST5C_4
	float v128; // [esp+18h] [ebp-118h]
	float v129; // [esp+18h] [ebp-118h]
	float v130; // [esp+1Ch] [ebp-114h]
	RwV3d in; // [esp+20h] [ebp-110h]
	RwV3d point; // [esp+2Ch] [ebp-104h]
	float v133; // [esp+38h] [ebp-F8h]
	float v134; // [esp+3Ch] [ebp-F4h]
	RwV3d outPoint; // [esp+40h] [ebp-F0h]
	CMatrix matrix; // [esp+4Ch] [ebp-E4h]
	CMatrix thisa; // [esp+94h] [ebp-9Ch]
	CMatrix v138; // [esp+DCh] [ebp-54h]
	int v139; // [esp+12Ch] [ebp-4h]

	bike = thisBike;
	if (thisBike->m_bPedRightHandFixed || (result = thisBike->m_bPedLeftHandFixed) != 0)
	{
		matrix.m_pAttachMatrix = 0;
		matrix.m_bOwnsAttachedMatrix = 0;
		v139 = 0;
		thisa.m_pAttachMatrix = 0;
		thisa.m_bOwnsAttachedMatrix = 0;
		v4 = thisBike->m_aBikeNodes[1];
		v139 = 1; //LOBYTE
		if (v4)
		{
			thisa.Attach(&v4->modelling, false);
			matrix.operator=(*bike->m_matrix);
			matrix.operator*=(thisa);

			v5 = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
			bikeID = bike->m_nModelIndex;
			clump = v5;
			//dummyBarPos = &CModelInfo::ms_modelInfoPtrs[bikeID]->m_pVehicleStruct->m_vDummyPos[10];

			CBaseModelInfo * bikeModelInfo = CModelInfo::GetModelInfo(bikeID);

			//dummyBarPos = &CModelInfo::ms_modelInfoPtrs[bikeID].vehicle
			
			point.x = dummyBarPos->x;
			point.y = dummyBarPos->y;
			point.z = dummyBarPos->z;
			if (point.x != 0.0 || point.y != 0.0 || point.z != 0.0)
			{
				boneIndex = RpHAnimIDGetIndex(clump, BONE_RIGHTWRIST);// we get the bone index
				boneMatrixArray = RpHAnimHierarchyGetMatrixArray(clump);// we obtain an array of bone matrices
				in = {-0.075000003, -0.029999999, 0.0};
				CMatrix::CMatrix(&v138, &boneMatrixArray[boneIndex], 0);// we create a matrix on the basis of a matrix from an array
				v139 = 2; //LOBYTE
				v11 = Multiply3x3(&outPoint, &v138, &in);
				in.x = v11->x;
				in.y = v11->y;
				in.z = v11->z;
				v12 = Multiply3x3(&outPoint, &in, &matrix);
				in.x = v12->x;
				in.y = v12->y;
				v13 = v12->z;
				point.x = in.x + point.x;
				in.z = v13;
				v139 = 1; //LOBYTE
				point.y = in.y + point.y;
				point.z = v13 + point.z;
				CMatrix::~CMatrix(&v138);
			}
			else if (bikeID == MODEL_MTBIKE)
			{
				point = {0.25, 0.28999999, 0.52499998};
			}
			else if (bikeID == MODEL_BIKE)
			{
				point = {0.25, 0.20999999, 0.69};
			}
			else
			{
				point = {0.25, 0.28999999, 0.52499998};
			}
			if (bike->m_bPedRightHandFixed)
			{
				v14 = operator*(&outPoint, &matrix, &point);
				v15 = v14->x;
				v16 = v14->y;
				v17 = v14->z;
				in.x = v15;
				in.y = v16;
				in.z = v17;
				v18 = RpHAnimIDGetIndex(clump, BONE_RIGHTWRIST);
				v19 = &RpHAnimHierarchyGetMatrixArray(clump)[v18].pos;
				v20 = in.z - v19->z;
				v21 = in.y - v19->y;
				v22 = in.x - v19->x;
				v23 = v19->z + v20;
				v24 = v19->y + v21;
				v25 = v22 + v19->x;
				v19->x = v25;
				v26 = v24;
				v19->y = v26;
				v27 = v23;
				v19->z = v27;
				v28 = RpHAnimIDGetIndex(clump, BONE_RIGHTHAND);
				v29 = &RpHAnimHierarchyGetMatrixArray(clump)[v28].pos;
				v30 = v29->z + v20;
				v31 = v29->y + v21;
				v32 = v29->x + v22;
				v29->x = v32;
				v33 = v31;
				v29->y = v33;
				v34 = v30;
				v29->z = v34;
				v35 = RpHAnimIDGetIndex(clump, BONE_RIGHTTHUMB);
				v36 = &RpHAnimHierarchyGetMatrixArray(clump)[v35].pos;
				v37 = v36->z + v20;
				v38 = v36->y + v21;
				v39 = v36->x + v22;
				v36->x = v39;
				v40 = v38;
				v36->y = v40;
				v41 = v37;
				v36->z = v41;
				v42 = RpHAnimIDGetIndex(clump, BONE_RIGHTELBOW);
				v43 = &RpHAnimHierarchyGetMatrixArray(clump)[v42].pos;
				v44 = v43->x;
				v134 = v21 * 0.667;
				v45 = v43->z;
				v46 = v43->y;
				outPoint.x = v22 * 0.667;
				v47 = v46 + v134;
				v48 = outPoint.x + v44;
				v43->x = v48;
				v49 = v47;
				v43->y = v49;
				v50 = v20 * 0.667 + v45;
				v43->z = v50;
				v51 = RpHAnimIDGetIndex(clump, BONE_RIGHTSHOULDER);
				v52 = v20 * 0.333;
				v53 = &RpHAnimHierarchyGetMatrixArray(clump)[v51].pos;
				v55 = v21 * 0.333;
				v56 = v53->x;
				v57 = v53->z;
				v128 = v55;
				v58 = v53->y;
				v134 = v55;
				v59 = v22 * 0.333;
				v133 = v59;
				outPoint.x = v59;
				v60 = v58 + v134;
				v61 = outPoint.x + v56;
				v53->x = v61;
				v62 = v60;
				v53->y = v62;
				v63 = v52 + v57;
				v53->z = v63;
				if (ped->IsPlayer)
				{
					v64 = RpHAnimIDGetIndex(clump, 301);
					v65 = &RpHAnimHierarchyGetMatrixArray(clump)[v64].pos;
					v66 = v65->x;
					v67 = v65->y;
					v54 = v52;
					v68 = v54 + v65->z;
					v134 = v128;
					outPoint.x = v133;
					v69 = v133 + v66;
					v65->x = v69;
					v70 = v67 + v128;
					v65->y = v70;
					v71 = v68;
					v65->z = v71;
				}
			}
			if (bike->m_bPedLeftHandFixed)
			{
				point.x = point.x * -1.0;
				v72 = operator*(&outPoint, &matrix, &point);
				v73 = v72->y;
				in.x = v72->x;
				v74 = v72->z;
				in.y = v73;
				in.z = v74;
				v75 = RpHAnimIDGetIndex(clump, 34);
				v76 = &RpHAnimHierarchyGetMatrixArray(clump)[v75].pos;
				v77 = in.z - v76->z;
				v78 = in.y - v76->y;
				v79 = in.x - v76->x;
				v80 = v76->z + v77;
				v81 = v76->y + v78;
				v82 = v79 + v76->x;
				v76->x = v82;
				v83 = v81;
				v76->y = v83;
				v84 = v80;
				v76->z = v84;
				v85 = RpHAnimIDGetIndex(clump, 35);
				v86 = &RpHAnimHierarchyGetMatrixArray(clump)[v85].pos;
				v87 = v86->z + v77;
				v88 = v86->y + v78;
				v89 = v86->x + v79;
				v86->x = v89;
				v90 = v88;
				v86->y = v90;
				v91 = v87;
				v86->z = v91;
				v92 = RpHAnimIDGetIndex(clump, 36);
				v93 = &RpHAnimHierarchyGetMatrixArray(clump)[v92].pos;
				v94 = v93->z + v77;
				v95 = v93->y + v78;
				v96 = v93->x + v79;
				v93->x = v96;
				v97 = v95;
				v93->y = v97;
				v98 = v94;
				v93->z = v98;
				v99 = RpHAnimIDGetIndex(clump, 33);
				v100 = &RpHAnimHierarchyGetMatrixArray(clump)[v99].pos;
				v101 = v100->x;
				v133 = v78 * 0.75;
				v102 = v100->z;
				v103 = v100->y;
				outPoint.x = v79 * 0.75;
				v104 = v103 + v133;
				v105 = outPoint.x + v101;
				v100->x = v105;
				v106 = v104;
				v100->y = v106;
				v107 = v77 * 0.75 + v102;
				v100->z = v107;
				v108 = RpHAnimIDGetIndex(clump, 32);
				v109 = v77 * 0.40000001;
				v110 = &RpHAnimHierarchyGetMatrixArray(clump)[v108].pos;
				v134 = v109;
				v111 = v78 * 0.40000001;
				v112 = v110->x;
				v113 = v110->y;
				v114 = v110->z;
				v129 = v111;
				v133 = v111;
				v115 = v79 * 0.40000001;
				v130 = v115;
				outPoint.x = v115;
				v116 = v113 + v133;
				v117 = outPoint.x + v112;
				v110->x = v117;
				v118 = v116;
				v110->y = v118;
				v119 = v109 + v114;
				v110->z = v119;
				if (ped->IsPlayer)
				{
					v120 = RpHAnimIDGetIndex(clump, 302);
					v121 = &RpHAnimHierarchyGetMatrixArray(clump)[v120].pos;
					v122 = v121->x;
					v123 = v121->y;
					v124 = v134 + v121->z;
					v133 = v129;
					outPoint.x = v130;
					v125 = v130 + v122;
					v121->x = v125;
					v126 = v123 + v129;
					v121->y = v126;
					v127 = v124;
					v121->z = v127;
				}
			}
			bike->m_bPedLeftHandFixed = 0;
			bike->m_bPedRightHandFixed = 0;
			v139 = 0; //LOBYTE
			CMatrix::~CMatrix(&thisa);
			v139 = -1;
			result = CMatrix::~CMatrix(&matrix);
		}
		else
		{
			v139 = 0; //LOBYTE
			CMatrix::~CMatrix(&thisa);
			v139 = -1;
			result = CMatrix::~CMatrix(&matrix);
		}
	}
	return;
}
*/