#include "GripForceTest.h"
#include <windows.h>
#include <conio.h>
#include <Springhead.h>
#include <HumanInterface/SprHIDRUsb.h>
#include <HumanInterface/SprHIKeyMouse.h>
#include <Foundation/SprUTQPTimer.h>
#include <iomanip>



//Constructor 
GripForceTest::GripForceTest(){

	humanInterface = SPIDAR;
	spidars[0] = NULL;
	spidars[1] = NULL;
	fileName = "./sprfiles/scene.spr";
	pdt = 0.001f;
	posScale = 10.0;  //2.5 orignal value with 20x30 floor scene (Virgilio original) (for peta pointer)  10.0 

	//Inits the debug CSV file
	//this->myfile.open("c:\\tmp\\loco.csv");

	displayGraphFlag = true;
}

//main function of the class
void GripForceTest::Init(int argc, char* argv[]){
	FWApp::Init(argc, argv);

	InitCameraView();

	BuildScene();
	sceneNumber = GetSdk()->NScene() - 1;
	GetWin(0)->SetScene(GetSdk()->GetScene(sceneNumber));
	InitHapticInterface();
		
	ptimer = CreateTimer(UTTimerIf::MULTIMEDIA);
	pTimerID = ptimer->GetID();
	//DSTR << "timer id xxx: " << pTimerID << std::endl;
	ptimer->SetResolution(1);
	ptimer->SetInterval(1);
	ptimer->Start();

	count = 0;
	delay = 0;
}
//This function loads the spr file and inits the scene
void GripForceTest::BuildScene(){
	int i = 0;
	UTRef<ImportIf> import = GetSdk()->GetFISdk()->CreateImport();	/// インポートポイントの作成
	GetSdk()->LoadScene(fileName, import);	/// ファイルのロード

	i = GetSdk()->NScene() - 1;
	phscene = GetSdk()->GetScene(i)->GetPHScene();
	phscene->SetTimeStep(pdt);

	fwscene = GetSdk()->GetScene(i);
	//fwscene->EnableRenderAxis();

	fingers[0].Build(0, fwscene);
	fingers[1].Build(1, fwscene);
	GetSdk()->GetScene(i)->EnableRenderHaptic();
	phscene->SetContactMode(fingers[0].tool, fingers[1].tool, PHSceneDesc::ContactMode::MODE_NONE);

	//PHHapticEngineIf* he = phscene->GetHapticEngine();	// 力覚エンジンをとってくる
	//PHHapticEngineDesc hd;
	//he->EnableHapticEngine(true);						// 力覚エンジンの有効化
	//he->SetHapticEngineMode(PHHapticEngineDesc::SINGLE_THREAD);
	this->nsolids = phscene->NSolids();
	DSTR << "Nsolids: " << nsolids << std::endl;  //DEBUG
	PHSolidIf **solidspnt = phscene->GetSolids();

	PHSolidIf *floor = phscene->FindObject("soCube")->Cast();
	floor->GetShape(0)->SetStaticFriction(0.4f);
	
/*
	//define jenga object properties
	fJenga1 = phscene->FindObject("soJenga1")->Cast();
	fJenga1->GetShape(0)->SetDensity(357.142f);  //non specific value try and error
	fJenga1->CompInertia();
	fJenga1->GetShape(0)->SetStaticFriction(0.7f);
	fJenga1->GetShape(0)->SetDynamicFriction(0.7f);
	//DSTR << "jenga mass: " << fJenga1->GetMass() << std::endl;  //debug
	//DSTR << "jenga volume: " << fJenga1->GetShape(0)->CalcVolume() << std::endl;  //debug
*/

	//defining the cellphone density and mass
/*
	fPhone = phscene->FindObject("soPhone")->Cast();
	fPhone->GetShape(0)->SetDensity(677);  // non specific value try and error
	fPhone->CompInertia();
	fPhone->GetShape(0)->SetStaticFriction(0.7f);
	fPhone->GetShape(0)->SetDynamicFriction(0.7f);
	DSTR << "phone mass: " << fPhone->GetMass() << std::endl;  //debug
	DSTR << "phone volume" << fPhone->GetShape(0)->CalcVolume() << std::endl;  //debug
	
	//defining the hammer density and mass
	fHammer = phscene->FindObject("soHammer")->Cast();
	fHammer->GetShape(0)->SetDensity(1500); //non specific value try and error  1012.25
	fHammer->GetShape(1)->SetDensity(244.90f);  //non specific value try and error
	fHammer->GetShape(0)->SetStaticFriction(0.70f);
	fHammer->GetShape(0)->SetDynamicFriction(0.7f);
	fHammer->GetShape(1)->SetStaticFriction(0.7f);
	fHammer->GetShape(1)->SetDynamicFriction(0.7f);
	fHammer->CompInertia();
	//DSTR << "hammer mass: " << fHammer->GetMass() << std::endl;  //debug
	//DSTR << "head volume: " << fHammer->GetShape(0)->CalcVolume() << std::endl;  //debug
	//DSTR << "stick volume: " << fHammer->GetShape(1)->CalcVolume() << std::endl; //debug 
*/
	//aluminium block  mass and density  64cm^3 * iron density (7.87gr) = 200
	fAluminio = phscene->FindObject("soAluminio")->Cast();
	fAluminio->GetShape(0)->SetDensity(1355);  //non specific value try and error
	fAluminio->CompInertia();
	fAluminio->GetShape(0)->SetStaticFriction(0.7f);
	fAluminio->GetShape(0)->SetDynamicFriction(0.7f);
	DSTR << "aluminio vol: " << fAluminio->GetShape(0)->CalcVolume() << std::endl;  //debug
	DSTR << "aluminio mass: " << fAluminio->GetMass() << std::endl;   //debug
}

//Inits SPIDAR and calibrates the pointer position
void GripForceTest::InitHapticInterface() {
	HISdkIf* hiSdk = GetSdk()->GetHISdk();

	bool bFoundCy = false;
	if (humanInterface == SPIDAR) {
		// x64
		DRCyUsb20Sh4Desc cyDesc;
		for (int i = 0; i < 10; ++i) {
			cyDesc.channel = i;
			DRCyUsb20Sh4If* cy = hiSdk->AddRealDevice(DRCyUsb20Sh4If::GetIfInfoStatic(), &cyDesc)->Cast();
			if (cy && cy->NChildObject()) {
				bFoundCy = true;
			}
			else {
				hiSdk->DelChildObject(cy);
			}
		}
		DRUARTMotorDriverDesc uartDesc;
		uartMotorDriver = hiSdk->AddRealDevice(DRUARTMotorDriverIf::GetIfInfoStatic(), &uartDesc)->Cast();
		hiSdk->AddRealDevice(DRKeyMouseWin32If::GetIfInfoStatic());
		hiSdk->Print(DSTR);
		hiSdk->Print(std::cout);

		spidars[0] = hiSdk->CreateHumanInterface(HISpidar4If::GetIfInfoStatic())->Cast();
		spidars[1] = hiSdk->CreateHumanInterface(HISpidar4If::GetIfInfoStatic())->Cast();

		//	モータの取り付け位置. モータが直方体に取り付けられている場合は，
		const float PX = 0.12f / 2;		//	x方向の辺の長さ/2
		const float PY = 0.14f / 2;		//	y方向の辺の長さ/2
		const float PZ = 0.12f / 2;		//	z方向の辺の長さ/2

		Vec3f motorPos[2][4] = {
			{ Vec3f(-PX, -PY, PZ), Vec3f(PX, -PY, -PZ), Vec3f(PX, PY, PZ), Vec3f(-PX, PY, -PZ) },
			{ Vec3f(PX, -PY, PZ), Vec3f(-PX, -PY, -PZ), Vec3f(-PX, PY, PZ), Vec3f(PX, PY, -PZ) },
		};
		Vec3f knotPos[4] = { Vec3f(), Vec3f(), Vec3f(), Vec3f() };
		HISpidar4Desc desc0;
		desc0.Init(4, motorPos[0], knotPos, 0.365296803653f, 1.66555e-5f, 0.3, 10.0);
		desc0.motors[1].lengthPerPulse *= -1;
		desc0.motors[2].lengthPerPulse *= -1;
		if (spidars[0]->Init(&desc0)) {
			spidars[0]->Calibration();
		}
		HISpidar4Desc desc1;
		desc1.Init(4, motorPos[1], knotPos, 0.365296803653f, 1.66555e-5f, 0.3, 10.0);
		desc1.motors[1].lengthPerPulse *= -1;
		desc1.motors[2].lengthPerPulse *= -1;
		if (spidars[1]->Init(&desc1)) {
			spidars[1]->Calibration();
		}
		hiSdk->Print(DSTR);
		hiSdk->Print(std::cout);
	}
}

void GripForceTest::InitCameraView(){

	Vec3d pos = Vec3d(0.04, 0.1, 0.3);
	GetCurrentWin()->GetTrackball()->SetPosition(pos);
	Affinef af;
	af.Pos() = pos;
	Vec3d target = Vec3d(0.0, 0.05, 0);	 //focused on the tochdown zone
	GetCurrentWin()->GetTrackball()->SetTarget(target);	// カメラ初期位置の設定
}


//Calibrates the position of the grip and both pointers
void GripForceTest::calibrate() {	
	spidars[0]->Calibration();
	spidars[1]->Calibration();
}

//This multimedia thread handles the haptic (6DOF virtual coupling pointers) and physics simulation (Springhead)
void GripForceTest::TimerFunc(int id){
	
	//DSTR << "timers id: " << pTimerID << std::endl;
	if (pTimerID == id){
		// Count "Cycle Per Second"
		if (1){
			static int cycle = 0;
			static DWORD lastCounted = timeGetTime();
			DWORD now = timeGetTime();
			if (now - lastCounted > 1000) {
			float cps = (float)(cycle) / (float)(now - lastCounted) * 1000.0f;
			std::cout << cps << std::endl;
			lastCounted = now;
			cycle = 0;
			int a = 0;
			}
			cycle++;
		}
		UTAutoLock LOCK(displayLock);

		phscene->Step();  //springhead physics step
		
		for (int i = 0; i < NFINGERS; ++i){
			int sidx = NFINGERS - i - 1;
			if (spidars[sidx]->IsGood()) {
				Finger& finger = fingers[i];
				Vec3f pos = spidars[sidx]->GetPosition();
				pos.y += 0.005;
				//pos.x += sidx ? 0.001 : -0.001;
				//if (i == 0) DSTR << "Spidar0Pos = " << pos << std::endl;
				finger.device->SetCenterPosition(4 * pos);
				Vec3d outForce = finger.CalcForce(bOutputVib);
				//if (i == 0) std::cout << "of:" << outForce << std::endl;
				if (outPutType) {
					if (outPutType == 1) {
						spidars[sidx]->SetForce(outForce);
					}
					else {
						spidars[sidx]->SetForce(Vec3d());
						spidars[sidx]->SetLimitMinForce(outForce.norm());
					}
				}
				else {
					spidars[sidx]->SetForce(Vec3d());
					spidars[sidx]->SetLimitMinForce(0.3);
				}
				spidars[sidx]->Update(pdt);  //updates the forces displayed in SPIDAR

/*	
				for (int m = 0; m < spidars[sidx]->NMotor(); ++m) {
					std::cout << spidars[sidx]->GetMotor(m)->GetLength() << " ";
				}
				if (i==1) std::cout << std::endl;	//	*/
			}
		}
				
		PostRedisplay();
	}
	else {
		return;
	}
}

//catches keyboard events
void GripForceTest::Keyboard(int key, int x, int y){
	if (ptimer){
		while (!ptimer->Stop());
	}
	int spKey = key - 0x100;
	switch (key){
		case 27:
		case 'q':
			exit(0);
			break;
		case 'g':
			GetSdk()->SetDebugMode(false);
			fwscene->EnableRenderPHScene(false);
			fwscene->EnableRenderGRScene(true);
			break;
		case 'h':
			GetSdk()->SetDebugMode(true);
			fwscene->EnableRenderGRScene(false);
			fwscene->EnableRenderPHScene(true);
			break;
		case 'w':
			InitCameraView();
			break;
		case 'c': {
			calibrate();
		}
			break;
		case 'f': {
			outPutType ++;
			if (outPutType == 3) {
				outPutType = 0;
			}
		}
				break;
		case 'v': {
			bOutputVib = !bOutputVib;
		}
				break;
		case 'd': {
			if (displayGraphFlag) {
				displayGraphFlag = false;
				DSTR << "GRAPH DISBLED" << std::endl;
			}else {
				displayGraphFlag = true;
				DSTR << "GRAPH ENABLED" << std::endl;
			}
		}
		case 's': {
			this->resetObjects();      
		}
			break;
		case '1': case '2': case '3': case '4': case '5': case '6': 
		case '7': case '8': case '9':
			grabKey = key; 

			break;
		
			//NUM KEYS BLOCK
		case 356: // left
		{

		}
			break;
		case 358: // right
		{
			

		}
			break;
		case 357: // up
		{
			
		}
			break;
		case 359: // down
		{
			
		}
			break;
	}
	
	ptimer->Start();
}

//draws the force graphic on the right of the screen
void GripForceTest::displayGraph(GRRenderIf* render)
{
	Affinef view; render->GetViewMatrix(view);
	Affinef proj; render->GetProjectionMatrix(proj);
	render->SetViewMatrix(Affinef::Unit());
	render->SetProjectionMatrix(Affinef::Unit());
	render->PushLight(ld);
	render->PushModelMatrix();
	render->SetModelMatrix(Affinef::Unit());

	render->SetMaterial(Spr::GRRenderBaseIf::TMaterialSample::WHITE);
	render->EnterScreenCoordinate();
	render->SetMaterial(GRRenderIf::WHITE);
	if (count != 0) {
		static double dif = 0.1; //0.4 orignal
		for (int n = 1; n<count-1; n++) {
			render->DrawLine(Vec3d((double(n) / VIBBUF_LEN) + dif, vibBuffer[n] / 10.0, 0), 
				Vec3d((double(n+1) / VIBBUF_LEN) + dif, vibBuffer[n+1] / 10.0, 0));
		}
	}

	render->LeaveScreenCoordinate();

	render->PopModelMatrix();
	render->PopLight();
	render->SetProjectionMatrix(proj);
	render->SetViewMatrix(view);
}

//Used to graphically debug the program
void GripForceTest::Display() 
{
	GRRenderIf* render = GetSdk()->GetRender();

	if (displayGraphFlag){
		displayGraph(render);
	}

	FWApp::Display();
}


void GripForceTest::AtExit(){
	//this->myfile.flush();
	//this->myfile.close();  //close the DEBUG csv file at exit
}

// Initialize the position of the objects in the scene
void GripForceTest::resetObjects(){

	Quaterniond qq;
	Posed ptmp;

	//left jenga
	if (fJenga1){
		fJenga1->SetVelocity(Vec3d());
		qq.FromEuler(Vec3f(Radf(90.0f), Radf(0.0f), 0.0f));
		ptmp = Posed(Vec3d(-0.121f, 0.021f, 0.1f), qq);
		fJenga1->SetPose(ptmp);
	}

	//middle jenga
	//fJenga2->SetVelocity(Vec3d());
	//qq.FromEuler(Vec3f(Radf(90.0f), Radf(0.0f), 0.0f));
	//ptmp = Posed(Vec3d(-0.1f, 0.021f, 0.1f), qq);
	//fJenga2->SetPose(ptmp);

	//right jenga
	//fJenga3->SetVelocity(Vec3d());
	//qq.FromEuler(Vec3f(Radf(90.0f), Radf(0.0f), 0.0f));
	//ptmp = Posed(Vec3d(-0.080f, 0.021f, 0.1f), qq);
	//fJenga3->SetPose(ptmp);

	//phone
	if (fPhone) {
		fPhone->SetVelocity(Vec3d());
		qq.FromEuler(Vec3f(Radf(90.0f), Radf(0.0f), 0.0f));
		ptmp = Posed(Vec3d(0.0f, 0.045f, 0.1f), qq);
		fPhone->SetPose(ptmp);
	}

	//hammer
	if (fHammer){
		fHammer->SetVelocity(Vec3d());
		qq.FromEuler(Vec3f(Radf(0.0f), Radf(180.0f), 0.0f));
		ptmp = Posed(Vec3d(0.1f, 0.045f, 0.1f), qq);
		fHammer->SetPose(ptmp);
	}

	//alumini cube
	fAluminio->SetVelocity(Vec3d());
	qq.FromEuler(Vec3f(Radf(0.0f), Radf(180.0f), 0.0f));
	ptmp = Posed(Vec3d(0.0f, 0.025f, -0.1f), qq);
	fAluminio->SetPose(ptmp);
}
