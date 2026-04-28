#include "pch.h"
#include "StartScene.h"
#include "SceneChanger.h"
#include "FBXRender.h"
#include "FBXRotateObj.h"
#include "UIButton.h"
#include "UILabel.h"

void StartScene::Init()
{
	SceneChanger::Create();
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
}

void StartScene::Release()
{
	SceneChanger::Destroy();
}

void StartScene::Start()
{
	Camera::GetInstance()->SetPos(0.0f, 0.0f);

	GameObject* gameButtonObj = new GameObject();
	UIButton* gameButton = new UIButton();
	gameButtonObj->AddComponent(gameButton);
	gameButtonObj->InitializeSet();
	gameButton->SetPosition(D3DXVECTOR2(40.0f, 40.0f));
	gameButton->SetSize(D3DXVECTOR2(220.0f, 90.0f));
	gameButton->SetUseTexture(false);
	gameButton->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 215, 235, 255), D3DCOLOR_ARGB(255, 180, 205, 230));
	gameButton->SetOrderInLayer(10);
	gameButton->SetOnClick(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

	GameObject* gameLabelObj = new GameObject();
	UILabel* gameLabel = new UILabel();
	gameLabelObj->AddComponent(gameLabel);
	gameLabelObj->InitializeSet();
	gameLabel->SetPosition(D3DXVECTOR2(68.0f, 72.0f));
	gameLabel->SetSize(D3DXVECTOR2(180.0f, 34.0f));
	gameLabel->SetText(L"GameScene Load");
	gameLabel->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	gameLabel->SetFontSize(22);
	gameLabel->SetOrderInLayer(20);

	GameObject* editButtonObj = new GameObject();
	UIButton* editButton = new UIButton();
	editButtonObj->AddComponent(editButton);
	editButtonObj->InitializeSet();
	editButton->SetPosition(D3DXVECTOR2(300.0f, 40.0f));
	editButton->SetSize(D3DXVECTOR2(220.0f, 90.0f));
	editButton->SetUseTexture(false);
	editButton->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 225, 240, 220), D3DCOLOR_ARGB(255, 190, 215, 185));
	editButton->SetOrderInLayer(10);
	editButton->SetOnClick(bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));

	GameObject* editLabelObj = new GameObject();
	UILabel* editLabel = new UILabel();
	editLabelObj->AddComponent(editLabel);
	editLabelObj->InitializeSet();
	editLabel->SetPosition(D3DXVECTOR2(338.0f, 72.0f));
	editLabel->SetSize(D3DXVECTOR2(170.0f, 34.0f));
	editLabel->SetText(L"EditScene Load");
	editLabel->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	editLabel->SetFontSize(22);
	editLabel->SetOrderInLayer(20);

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


	fbxSample2->SetParent(fbxSample1);


	GameObject* test1 = new GameObject();
	test1->SetTag("TEST_1");
	test1->InitializeSet();
	GameObject* test2 = new GameObject();
	test2->SetTag("TEST_2");
	test2->InitializeSet();

	test1->AddChild(test2);

}
