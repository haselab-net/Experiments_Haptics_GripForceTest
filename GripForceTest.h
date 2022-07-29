#ifndef GRIPFORCE_TEST_H
#define GRIPFORCE_TEST_H

//FLEXI FORCE CALIBRATION PARAMETERS
#define SATURATION_VOLTAGE 0.624
#define NO_LOAD_VOLTAGE    1.763
#define NO_LOAD_NEWTON     0
#define SATURATION_NEWTON  4.4

//force display parameters
#define VIBBUF_LEN 100

//defines the angle detection range (in +,- degrees)
#define ANGLE_RANGE 20

//number of jenga objects included in the sprfile 
#define JENGA_NUMBER 3

#include <Springhead.h>
#include <Framework\SprFWApp.h>
#include <HumanInterface\SprHIDRUsb.h>
#include <HumanInterface/SprHIKeyMouse.h>

#include <iomanip>

#include "FingerGrip.h"

using namespace Spr;
using namespace std;

class GripForceTest : public FWApp{
public: //Local Properties
	enum HumanInterface{
		SPIDAR,
		XBOX,
		FALCON,
	} humanInterface;

	#define  NFINGERS	2
	Finger fingers[NFINGERS];
	HISpidar4If* spidars[2];
	PHSceneIf* phscene;
	FWSceneIf* fwscene;
	DVAdIf* flexiforce;
	GRLightDesc ld;    //lights used to draw letters and the force grap in the screen

	UTRef<UTTimerIf> ptimer;
	int pTimerID;
	string fileName;
	int sceneNumber;
	float pdt;
	int nsolids;
	double posScale;  //2.5 orignal value with 20x30 floor scene (Virgilio original)
	int grabKey;	//	'1' to '9'


	GripForceTest();
	void InitHapticInterface();
	void Init(int argc, char* argv[]);
	void BuildScene();
	void InitCameraView();
	virtual void TimerFunc(int id);
	virtual void Keyboard(int key, int x, int y);
	virtual void Display(); //virtual fucntion used to display graphics
	virtual void AtExit();  //used to close the debug file
	void GripForceTest::GripForceTestStep(Vec3f* spidarForce);
	
	//phsolid objects assigned during buildscene()
	PHSolidIf *fPhone;
	//Binod Define phsolid objects
	PHSolidIf* fTool0;
	PHSolidIf* fTool1;
	PHSolidIf* fTool2;
	PHSolidIf *fHammer;
	PHSolidIf *fJenga1;
	//PHSolidIf *fJenga2;
	//PHSolidIf *fJenga3;
	PHSolidIf *fAluminio;

	//Used GripForceTest manipulation methods (Binod)
	Posed defaultCenterPose;
	Posed defaultPose1;
	Posed defaultPose2;
	Posed defaultPose3;
	double maxReach;
	double grabForce;
	double grabForce0;
	double grabForce1;
	double grabForce2;

	double vibBuffer[1000];
	int count;  //used to fill the buffer of the force graph
	int delay;  //used to fill the buffer of the force graph

	void calibrate(); //calibrates SPIDAR and orientates the pointers
	void displayGraph(GRRenderIf* render);  //draws the force graphic on the right of the screen

	void resetObjects();  //resets the objects position
	bool displayGraphFlag;  //flag to display the force sensor graph
	int outPutType = 0;
	bool bOutputVib = false;

	//BINOD objects
	DRUARTMotorDriverIf* uartMotorDriver;
};

#endif
