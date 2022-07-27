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
	int index = -1;
	int GetIndex() { return index; }
	void Build(FWSceneIf* fwScene);
	void SetDevicePosition(Vec3d pos);
protected:
	friend class FingerGrip;
};

#endif