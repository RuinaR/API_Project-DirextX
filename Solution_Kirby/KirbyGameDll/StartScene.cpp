#include "pch.h"
#include "StartScene.h"
#include "SceneChanger.h"
#include "FBXRender.h"
#include "FBXRotateObj.h"
#include "UIButton.h"
#include "UILabel.h"
#include "UIActionRegistry.h"

void StartScene::Init()
{
	Camera::GetInstance()->SetPos(0.0f, 0.0f);
	SceneChanger::Create();
	UIActionRegistry::RegisterAction("ChangeGameScene", bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));
	UIActionRegistry::RegisterAction("ChangeEditScene", bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
}

void StartScene::Release()
{
	UIActionRegistry::UnregisterAction("ChangeGameScene");
	UIActionRegistry::UnregisterAction("ChangeEditScene");
	SceneChanger::Destroy();
}

void StartScene::Start()
{
}

const char* StartScene::GetSceneName() const
{
	return "StartScene";
}

void StartScene::BuildInitialSceneObjects()
{
	Camera::GetInstance()->SetPos(0.0f, 0.0f);

	GameObject* gameButtonObj = new GameObject();
	UIButton* gameButton = new UIButton();
	gameButtonObj->AddComponent(gameButton);
	gameButtonObj->InitializeSet();
	D3DXVECTOR2 gameButtonPosition(40.0f, 40.0f);
	D3DXVECTOR2 gameButtonSize(220.0f, 90.0f);
	gameButton->SetPosition(&gameButtonPosition);
	gameButton->SetSize(&gameButtonSize);
	gameButton->SetUseTexture(false);
	gameButton->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 215, 235, 255), D3DCOLOR_ARGB(255, 180, 205, 230));
	gameButton->SetOrderInLayer(10);
	gameButton->SetActionKey("ChangeGameScene");
	gameButton->SetOnClick(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

	GameObject* gameLabelObj = new GameObject();
	UILabel* gameLabel = new UILabel();
	gameLabelObj->AddComponent(gameLabel);
	gameLabelObj->InitializeSet();
	D3DXVECTOR2 gameLabelPosition(68.0f, 72.0f);
	D3DXVECTOR2 gameLabelSize(180.0f, 34.0f);
	gameLabel->SetPosition(&gameLabelPosition);
	gameLabel->SetSize(&gameLabelSize);
	gameLabel->SetText(L"GameScene Load");
	gameLabel->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	gameLabel->SetFontSize(22);
	gameLabel->SetOrderInLayer(20);

	GameObject* editButtonObj = new GameObject();
	UIButton* editButton = new UIButton();
	editButtonObj->AddComponent(editButton);
	editButtonObj->InitializeSet();
	D3DXVECTOR2 editButtonPosition(300.0f, 40.0f);
	D3DXVECTOR2 editButtonSize(220.0f, 90.0f);
	editButton->SetPosition(&editButtonPosition);
	editButton->SetSize(&editButtonSize);
	editButton->SetUseTexture(false);
	editButton->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 225, 240, 220), D3DCOLOR_ARGB(255, 190, 215, 185));
	editButton->SetOrderInLayer(10);
	editButton->SetActionKey("ChangeEditScene");
	editButton->SetOnClick(bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));

	GameObject* editLabelObj = new GameObject();
	UILabel* editLabel = new UILabel();
	editLabelObj->AddComponent(editLabel);
	editLabelObj->InitializeSet();
	D3DXVECTOR2 editLabelPosition(338.0f, 72.0f);
	D3DXVECTOR2 editLabelSize(170.0f, 34.0f);
	editLabel->SetPosition(&editLabelPosition);
	editLabel->SetSize(&editLabelSize);
	editLabel->SetText(L"EditScene Load");
	editLabel->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	editLabel->SetFontSize(22);
	editLabel->SetOrderInLayer(20);

	// FBX 테스트용 오브젝트

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
