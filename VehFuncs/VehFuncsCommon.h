#pragma once
#include "plugin.h"
#include "MatrixBackup.h"
#include <time.h>

// Mod funcs
#include "Footpegs.h"
#include "Anims.h"

using namespace plugin;
using namespace std;
using namespace injector;

extern unsigned int FramePluginOffset;

#define PLUGIN_ID_STR 'VEHF'
#define PLUGIN_ID_NUM 0x56454846

#define FRAME_EXTENSION(frame) ((FramePlugin *)((unsigned int)frame + FramePluginOffset))

CVector* GetVehicleDummyPosAdapted(CVehicle* vehicle, int dummyId);


struct FramePlugin
{
	// plugin data
	MatrixBackup * origMatrix;
	CVehicle *owner;
	int8_t LODdist;
	struct
	{
		unsigned char bNeverRender : 1;
		unsigned char bIsVarWheel : 1;
		unsigned char bDestroyOnRemoveUpgrade : 1;
	} flags;

	// plugin interface
	static RwFrame * Init(RwFrame *frame)
	{
		FRAME_EXTENSION(frame)->owner = nullptr;
		FRAME_EXTENSION(frame)->origMatrix = nullptr;
		FRAME_EXTENSION(frame)->flags.bNeverRender = false;
		FRAME_EXTENSION(frame)->flags.bIsVarWheel = false;
		FRAME_EXTENSION(frame)->flags.bDestroyOnRemoveUpgrade = false;
		FRAME_EXTENSION(frame)->LODdist = -100;
		return frame;
	}

	static RwFrame * Destroy(RwFrame *frame)
	{
		if (FRAME_EXTENSION(frame)->origMatrix != nullptr)
		{
			delete FRAME_EXTENSION(frame)->origMatrix;
		}
		return frame;
	}

	static RwFrame * Copy(RwFrame *copy, RwFrame *frame)
	{
		return copy;
	}
};

class ExtendedData {
public:

	// Process flags
	bool nodesProcess;
	bool nodesProcessForIndieHandling;
	bool nodesProcessInRender;

	// Management
	int randomSeed;
	int randomSeedUsage;
	RwFrame *classFrame;

	// Characteristics
	int8_t paintjob;
	int32_t driverModel;
	list<int> occupantsModels;
	uint8_t passAddChance;
	int8_t doubleExhaust;
	int8_t swingingChassis;
	float bodyTilt;
	float dirtyLevel;
	int32_t color[4];
	RwFrame *wheelFrame[6];

	// Utilities
	float realisticSpeed;
	float dotLife;
	float smoothGasPedal;
	float smoothBrakePedal;
	float brakeHeat;
	double kms;

	// Coplight / Taxilight
	RwFrame *taxilightFrame;
	RwFrame *coplightFrame;
	RwFrame *coplightoutFrame;

	// MDPM
	int16_t mdpmCustomChances;
	float mdpmCustomMinVol;
	float mdpmCustomMaxVol;

	//Soundize
	int backfireMode;

	// Speedometer
	RwFrame * speedoFrame;
	float speedoMult;
	RwFrame * speedoDigits[3];
	RpAtomic * speedoAtomics[10];

	// Odometer
	RwFrame * odometerFrame;
	double odometerMult;
	bool odometerHideZero;
	RwFrame * odometerDigits[9];
	RpAtomic * odometerAtomics[10];

	// Materials
	RpMaterial *taxiSignMaterial;
	RpMaterial *brakeDiscMaterial;

	// Popup lights
	float popupProgress[2];
	RwFrame * popupFrame[2];

	// Flags
	struct
	{
		unsigned char bBusRender : 1;
		unsigned char bDamageUpdated : 1;
		unsigned char bUpgradesUpdated : 1;
		unsigned char bPreservePaintjobColor : 1;
		unsigned char bPreservePaintjobColorButNotForever : 1;
		unsigned char bWasRenderedOnce : 1;
	} flags;

	RwFrame *triforkFrame;
	RwFrame *hitchFrame;
	list<RwFrame*> gearFrame;
	list<RwFrame*> fanFrame;
	list<RwFrame*> shakeFrame;
	list<RwFrame*> gaspedalFrame;
	list<RwFrame*> brakepedalFrame;
	list<F_footpegs*> fpegFront;
	list<F_footpegs*> fpegBack;
	list<RwFrame*> spoilerFrames;
	list<F_an*> anims;
	list<RwFrame*> steer;

	//Backfire
	FxSystem_c* backfire[2];
	FxSystem_c* backfireHigh[2];
	RwMatrix* backfireMatrix[2];
	RwMatrix* backfireMatrixHigh[2];
	
	// ---- Init
	ExtendedData(CVehicle *vehicle)
	{
		// Process flags
		nodesProcess = true;
		nodesProcessForIndieHandling = true;

		// Management
		randomSeed = rand();
		randomSeedUsage = 0;
		classFrame = nullptr;

		// Characteristics and misc features
		paintjob = -1;
		driverModel = -1;
		occupantsModels.clear();
		passAddChance = 0;
		dirtyLevel = -1;
		bodyTilt = 0.0f;
		color[0] = -1;
		color[1] = -1;
		color[2] = -1;
		color[3] = -1;
		memset(wheelFrame, 0, sizeof(wheelFrame));
		doubleExhaust = -1;
		swingingChassis = -1;

		// Utilities
		realisticSpeed = 0.0f;
		dotLife = 1.0f;
		smoothGasPedal = 0.0f;
		smoothBrakePedal = 0.0f;
		brakeHeat = 0.0f;
		kms = -1.0;

		// Coplight / Taxilight
		taxilightFrame = nullptr;
		coplightFrame = nullptr;
		coplightoutFrame = nullptr;

		// MDPM
		mdpmCustomChances = -1;
		mdpmCustomMinVol = 0.0f;
		mdpmCustomMaxVol = 0.0f;

		//Soundize
		backfireMode = -1;

		// Speedometer
		speedoFrame = nullptr;
		speedoMult = 1.0f;
		memset(speedoDigits, 0, sizeof(speedoDigits));
		memset(speedoAtomics, 0, sizeof(speedoAtomics));

		// Odometer
		odometerFrame = nullptr;
		odometerMult = 1.0;
		memset(odometerDigits, 0, sizeof(odometerDigits));
		memset(odometerAtomics, 0, sizeof(odometerAtomics));
		odometerHideZero = false;

		// Flags
		flags.bBusRender = 0;
		flags.bDamageUpdated = 0;
		flags.bUpgradesUpdated = 0;
		flags.bPreservePaintjobColor = 0;
		flags.bPreservePaintjobColorButNotForever = 0;
		flags.bWasRenderedOnce = 0;

		// Popup lights
		popupProgress[0] = 0.0f;
		popupProgress[1] = 0.0f;
		popupFrame[0] = nullptr;
		popupFrame[1] = nullptr;
		triforkFrame = nullptr;

		hitchFrame = nullptr;
		taxiSignMaterial = nullptr;
		brakeDiscMaterial = nullptr;

		// Lists
		gearFrame.clear();
		fanFrame.clear();
		shakeFrame.clear();
		gaspedalFrame.clear();
		brakepedalFrame.clear();
		fpegFront.clear();
		fpegBack.clear();
		spoilerFrames.clear();
		anims.clear();
		steer.clear();

		//Backfire
		for (int i = 0; i < 2; i++)
		{
			backfire[i] = nullptr;
			backfireHigh[i] = nullptr;
			backfireMatrix[i] = nullptr;
			backfireMatrixHigh[i] = nullptr;
		}
	}
	 
	// ---- Destroy
	~ExtendedData()
	{
		if (!anims.empty())
		{
			for (auto it : anims) delete it;
		}
		if (!fpegFront.empty())
		{
			for (auto it : fpegFront) delete it;
		}
		if (!fpegBack.empty())
		{
			for (auto it : fpegBack) delete it;
		}
		//Backfire
		for (int i = 0; i < 2; i++)
		{
			if (backfire[i]) backfire[i]->Kill();
			if (backfireHigh[i]) backfireHigh[i]->Kill();
			if (backfireMatrix[i]) delete backfireMatrix[i];
			if (backfireMatrixHigh[i]) delete backfireMatrixHigh[i];
		}
	}

	// ---- ReInit
	// Used mainly for Tuning Mod updating
	void ReInitForReSearch()
	{
		bodyTilt = 0.0f;
		mdpmCustomChances = -1;
		speedoFrame = nullptr;
		taxilightFrame = nullptr;
		coplightFrame = nullptr;
		coplightoutFrame = nullptr;
		speedoMult = 1.0;
		memset(&flags, 0, sizeof(flags));
		memset(speedoDigits, 0, sizeof(speedoDigits));
		memset(speedoAtomics, 0, sizeof(speedoAtomics));

		popupFrame[0] = nullptr;
		popupFrame[1] = nullptr;
		hitchFrame = nullptr;
		taxiSignMaterial = nullptr;
		triforkFrame = nullptr;

		gearFrame.clear();
		fanFrame.clear();
		shakeFrame.clear();
		gaspedalFrame.clear();
		brakepedalFrame.clear();
		fpegFront.clear();
		fpegBack.clear();
		spoilerFrames.clear();
		anims.clear();
		steer.clear();
	}
};

extern list<string> &getClassList();
extern VehicleExtendedData<ExtendedData> xData;
extern fstream lg;
extern bool useLog;
