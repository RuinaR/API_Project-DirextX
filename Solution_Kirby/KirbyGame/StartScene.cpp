#include "pch.h"
#include "StartScene.h"
#include "ColorButton.h"
#include "SceneChanger.h"

void StartScene::Init()
{
	SceneChanger::Create();
	m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG");
}

void StartScene::Release()
{
	SceneChanger::Destroy();
	AnimationManager::ReleaseTexture(m_bg);
}

void StartScene::Start()
{
	GameObject* btnObj = new GameObject();
	ColorButton* btn = new ColorButton();
	btnObj->AddComponent(btn);
	btnObj->SetOrderInLayer(10);
	btnObj->InitializeSet();
	btn->SetUIPos({ 0,0, 2.0f });
	btn->SetUISize({ 200,100 });
	btn->SetText(TEXT("GameScene Load"));
	btn->SetTextColor(RGB(255, 0, 255));
	btn->SetDefaultColor(RGB(255, 255, 255));
	btn->SetHoverColor(RGB(200, 200, 200));
	btn->SetDownColor(RGB(150, 150, 150));
	btn->SetTextSize(20);
	btn->SetEvent(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

	GameObject* btnObj2 = new GameObject();
	ColorButton* btn2 = new ColorButton();
	btnObj2->AddComponent(btn2);
	btnObj2->SetOrderInLayer(10);
	btnObj2->InitializeSet();
	btn2->SetUIPos({ 200,100, 2.0f });
	btn2->SetUISize({ 200,100 });
	btn2->SetText(TEXT("EditScene Load"));
	btn2->SetTextColor(RGB(255, 0, 255));
	btn2->SetDefaultColor(RGB(255, 255, 255));
	btn2->SetHoverColor(RGB(200, 200, 200));
	btn2->SetDownColor(RGB(150, 150, 150));
	btn2->SetTextSize(20);
	btn2->SetEvent(bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));
}
