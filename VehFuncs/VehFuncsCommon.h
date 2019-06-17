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

struct FramePlugin
{
	// plugin data
	MatrixBackup * origMatrix;

	// plugin interface
	static RwFrame * Init(RwFrame *frame)
	{
		FRAME_EXTENSION(frame)->origMatrix = nullptr;
		return frame;
	}

	static RwFrame * Destroy(RwFrame *frame)
	{
		if (FRAME_EXTENSION(frame)->origMatrix)
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
	bool nodesProcessed;
	bool nodesProcessedSecFrame;
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
	float bodyTilt;
	float dirtyLevel;
	int8_t doubleExhaust;
	int32_t color[4];
	RwFrame *wheelFrame[6];

	// Utilities
	float dotLife;
	float smoothGasPedal;
	float smoothBrakePedal;

	// Coplight
	RwFrame *coplightFrame;
	RwFrame *coplightoutFrame;

	// MDPM
	int16_t mdpmCustomChances;
	float mdpmCustomMinVol;
	float mdpmCustomMaxVol;

	// Speedometer
	RwFrame * speedoFrame;
	float speedoMult;
	RwFrame * speedoDigits[3];
	RpAtomic * speedoAtomics[10];

	// Taxi sign material
	RpMaterial *taxiSignMaterial;

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
	} flags;

	RwFrame *triforkFrame;
	RwFrame * hitchFrame;
	list<RwFrame*> gearFrame;
	list<RwFrame*> fanFrame;
	list<RwFrame*> shakeFrame;
	list<RwFrame*> gaspedalFrame;
	list<RwFrame*> brakepedalFrame;
	list<F_footpegs*> fpegFront;
	list<F_footpegs*> fpegBack;
	list<RwFrame*> spoilerFrames;
	list<F_an*> anims;
	
	// ---- Init
	ExtendedData(CVehicle *vehicle)
	{

		// Process flags
		nodesProcessed = false;
		nodesProcessedSecFrame = false;
		nodesProcessInRender = true;

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
		doubleExhaust = -1;
		bodyTilt = 0.0f;
		color[0] = -1;
		color[1] = -1;
		color[2] = -1;
		color[3] = -1;
		memset(wheelFrame, 0, sizeof(wheelFrame));

		// Utilities
		dotLife = 1.0f;
		smoothGasPedal = 0.0f;
		smoothBrakePedal = 0.0f;

		// Coplight
		coplightFrame = nullptr;
		coplightoutFrame = nullptr;

		// MDPM
		mdpmCustomChances = -1;
		mdpmCustomMinVol = 0.0f;
		mdpmCustomMaxVol = 0.0f;

		// Speedometer
		speedoFrame = nullptr;
		speedoMult = 1.0;
		memset(speedoDigits, 0, sizeof(speedoDigits));
		memset(speedoAtomics, 0, sizeof(speedoAtomics));

		// Flags
		flags.bBusRender = 0;

		// Popup lights
		popupProgress[0] = 0.0f;
		popupProgress[1] = 0.0f;
		popupFrame[0] = nullptr;
		popupFrame[1] = nullptr;
		triforkFrame = nullptr;

		hitchFrame = nullptr;
		taxiSignMaterial = nullptr;

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
	}

	// ---- ReInit
	// Used mainly for Tuning Mod updating
	void ReInitForReSearch()
	{
		bodyTilt = 0.0f;
		mdpmCustomChances = -1;
		speedoFrame = nullptr;
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
	}
};

extern fstream &logfile();
extern list<string> &getClassList();
extern VehicleExtendedData<ExtendedData> remInfo;
extern fstream lg;
