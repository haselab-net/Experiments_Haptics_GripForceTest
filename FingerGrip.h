#ifndef FINGER_GRIP_H
#define FINGER_GRIP_H
#include <Springhead.h>

using namespace Spr;
using namespace std;

struct FingerDesc {
	Vec3d direction;
	Vec3d position;
	PHSolidIf* device = NULL;		//	device's solid (should not have a shape)
	PHSolidIf* tool=NULL;			//	Tool's solid (should have a shape)
	PHSpringIf* spring = NULL;		//	slider joint from the device to the tool.
};

class Finger: public FingerDesc {
	int index = -1;
public:
	int GetIndex() { return index; }
	FingerDesc* GetDesc() { return this; }
	void SetDesc(FingerDesc* desc) { *(FingerDesc*)this = *desc; }
	void Build(FWSceneIf* fwScene);
	void SetDevicePosition(Vec3d pos);
protected:
	friend class FingerGrip;
};

#endif