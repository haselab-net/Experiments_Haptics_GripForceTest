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
	tool->SetInertia(tool->GetInertia() * 1000);

	PHSpringDesc sprDesc;
	sprDesc.spring = 500 * Vec3d(1, 1, 1);	//	0.5N/mm = 500N/m
	sprDesc.damper = sprDesc.spring * 0.001;
	sprDesc.springOri = sprDesc.spring[0];
	sprDesc.damperOri = sprDesc.damper[0];
	spring = phScene->CreateJoint(device, tool, sprDesc)->Cast();
}
Vec3d Finger::CalcForce(bool bVib) {
	Vec6d couplingForce = spring->GetMotorForce();
	Vec3d f = couplingForce.sub_vector(0, Vec3d());
	
	PHSceneIf* phs = tool->GetScene()->Cast();
	int nc = phs->NContacts();
	int nContact = 0;
	int nSlip = 0;
	for (int i = 0; i < nc; ++i) {
		PHContactPointIf* c = phs->GetContact(i);
		if (c->GetSocketSolid() == tool || c->GetPlugSolid() == tool) {
			nContact++;	
			if (!c->IsStatic()) {
				nSlip++;
			}
		}
	}
	if (nonContactCount > 10 && nContact) {
		time = 0;
	}
	if (nonSlipCount > 10 && nSlip == nContact) {
		time = 0.02;
	}
	double amp = 0;
	time += dt;
	if (time < 1000) {
		amp = A * cos(time *2*M_PI*Freq) * exp(-B * time);
	}
	else {
		amp = 0;
	}

	lastContact = nContact;
	if (nContact) {
		nonContactCount = 0;
	}
	else {
		nonContactCount++;
	}
	if (nSlip==nContact || !nContact) {
		nonSlipCount = 0;
	}
	else {
		nonSlipCount++;
	}


	Vec3d dir = f.square() > 0.001 ? f.unit() : Vec3f(0, 1, 0);

	double fs = 0.2, ts = 1;
	Vec3d outForce = -fs * f;
	if (bVib) outForce += amp * dir;
	return outForce;
}