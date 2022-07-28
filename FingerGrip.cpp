#include "FingerGrip.h"

void Finger::Build(int idx, FWSceneIf* fwScene) {
	PHSceneIf* phScene = fwScene->GetPHScene();

	device = phScene->CreateSolid();
	device->SetDynamical(false);
	device->SetPose(Vec3d(0.1 * (idx * 1 - 0.5), 0, 0));

	ostringstream toolName;
	toolName << "soTool" << idx;
	tool = phScene->FindObject(toolName.str().c_str())->Cast();
	if (!tool) {
		tool = fwScene->GetPHScene()->CreateSolid();
	}
	tool->SetGravity(false);
	tool->SetPose(device->GetPose());

	PHSpringDesc sprDesc;
	sprDesc.spring = 500 * Vec3d(1, 1, 1);	//	0.5N/mm = 500N/m
	sprDesc.damper = sprDesc.spring * 0.001;
	sprDesc.springOri = sprDesc.spring[0];
	sprDesc.damperOri = sprDesc.damper[0];
	spring = phScene->CreateJoint(device, tool, sprDesc)->Cast();
}
