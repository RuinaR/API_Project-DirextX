#include "pch.h"
#include "StartScene.h"
#include "Button.h"
#include "SceneChanger.h"

void StartScene::Init()
{
	SceneChanger::Create();
	m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG.bmp");
}

void StartScene::Release()
{
	SceneChanger::Destroy();
	AnimationManager::ReleaseTexture(m_bg);
}

void StartScene::Start()
{
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
}