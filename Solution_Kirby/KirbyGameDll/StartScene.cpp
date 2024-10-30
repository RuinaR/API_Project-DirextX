#include "pch.h"
#include "StartScene.h"
#include "Button.h"
#include "SceneChanger.h"
#include "FBXRender.h"
#include "FBXRotateObj.h"

void StartScene::Init()
{
	SceneChanger::Create();
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
}

void StartScene::Release()
{
	SceneChanger::Destroy();
	AnimationManager::ReleaseTexture(m_bg);
}

void StartScene::Start()
{
	Camera::GetInstance()->SetPos(0.0f, 0.0f);

	GameObject* btnObj = new GameObject();
	Button* btn = new Button();
	btnObj->AddComponent(btn);
	btnObj->InitializeSet();
	btn->SetUIPos({ -300,0, -1.0f });
	btn->SetUISize({ 200,100 });
	btn->SetText("GameScene Load");
	btn->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	btn->SetEvent(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

	GameObject* btnObj2 = new GameObject();
	Button* btn2 = new Button();
	btnObj2->AddComponent(btn2);
	btnObj2->InitializeSet();
	btn2->SetUIPos({ 100,0, -1.0f });
	btn2->SetUISize({ 200,100 });
	btn2->SetText("EditScene Load");
	btn2->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	btn2->SetEvent(bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));

	//FBX TEST

	GameObject* fbxSample1 = new GameObject();
	FBXRender* fbxRender1 = new FBXRender("fbxSample1");
	fbxSample1->SetAngleX(0.0f);
	fbxSample1->SetAngleY(0.0f);
	fbxSample1->SetAngleZ(0.0f);
	fbxSample1->SetTag("3D_OBJ");
	fbxSample1->SetPosition(D3DXVECTOR3(300.0f, 0.0f, 500.0f));
	fbxSample1->Size3D() = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	fbxSample1->AddComponent(fbxRender1);
	fbxSample1->AddComponent(new FBXRotateObj());
	fbxSample1->InitializeSet();


	GameObject* fbxSample2 = new GameObject();
	FBXRender* fbxRender2 = new FBXRender("fbxSample");
	fbxSample2->SetAngleX(00.0f);
	fbxSample2->SetAngleY(0.0f);
	fbxSample2->SetAngleZ(0.0f);
	fbxSample2->SetTag("3D_OBJ");
	fbxSample2->SetPosition(D3DXVECTOR3(-300.0f, 0.0f, 500.0f));
	fbxSample2->Size3D() = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	fbxSample2->AddComponent(fbxRender2);
	fbxSample2->AddComponent(new FBXRotateObj());
	fbxSample2->InitializeSet();
}
