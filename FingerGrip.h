#ifndef FINGER_GRIP_H
#define FINGER_GRIP_H
#include <Springhead.h>

using namespace Spr;
using namespace std;

class Finger{
public:
	PHSolidIf* device = NULL;		//	device's solid (should not have a shape)
	PHSolidIf* tool = NULL;			//	Tool's solid (should have a shape)
	PHSpringIf* spring = NULL;		//	slider joint from the device to the tool.
	void Build(int idx, FWSceneIf* fwScene);
	Vec3d CalcForce(bool bVib);
	void SetDevicePosition(Vec3d pos);
protected:
	friend class FingerGrip;
	int lastContact = 0;
	int nonContactCount = 0;
	int nonSlipCount = 0;
	double time = 1000;
	const double dt = 0.001;
	double A = 3;
	double B = 100;
	double Freq = 200;
};

#endif