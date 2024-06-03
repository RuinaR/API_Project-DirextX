#include "pch.h"
#include "StartScene.h"
#include "ColorButton.h"
#include "SceneChanger.h"

void StartScene::Init()
{
	SceneChanger::Create();
	m_bg = AnimationManager::LoadHBitmap("Bitmaps\\obj\\BG");
}

void StartScene::Release()
{
	SceneChanger::Destroy();
	AnimationManager::ReleaseHBitmap(m_bg);
}

void StartScene::Start()
{
	WindowFrame::GetInstance()->GetBuffer()->SetBG(m_bg);

	GameObject* btnObj = new GameObject();
	ColorButton* btn = new ColorButton();
	btnObj->AddComponent(btn);
	btnObj->SetOrderInLayer(10);
	btnObj->InitializeSet();
	btn->SetUIPos({ 500,400 });
	btn->SetUISize({ 200,50 });
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
	btn2->SetUIPos({ 800,400 });
	btn2->SetUISize({ 200,50 });
	btn2->SetText(TEXT("EditScene Load"));
	btn2->SetTextColor(RGB(255, 0, 255));
	btn2->SetDefaultColor(RGB(255, 255, 255));
	btn2->SetHoverColor(RGB(200, 200, 200));
	btn2->SetDownColor(RGB(150, 150, 150));
	btn2->SetTextSize(20);
	btn2->SetEvent(bind(&SceneChanger::ChangeEditScene, SceneChanger::GetInstance()));
}
