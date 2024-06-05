#include "pch.h"
#include "GameScene.h"
#include "Component.h"
#include "GameObject.h"
#include "Player.h"
#include "ChangeObject.h"
#include "StageMaker.h"
#include "ColorButton.h"
#include "DebugWindow.h"
#include "SceneChanger.h"

void GameScene::StartGame()
{
	cout << "Start Game" << endl;
	if (m_input == nullptr)
		return;
	string mapName = m_input->GetString();
	if (mapName.empty())
	{
		MessageBox(WindowFrame::GetInstance()->GetHWND(), TEXT("맵 이름을 입력 하세요"), TEXT("알림"), MB_OK);
		return;
	}
	m_btn->GetGameObject()->SetDestroy(true);
	m_input->GetGameObject()->SetDestroy(true);
	
	if (!StageMaker::GetInstance()->SetMap(mapName))
	{
		MessageBox(WindowFrame::GetInstance()->GetHWND(), TEXT("존재하지 않는 맵"), TEXT("알림"), MB_OK);
	}
	StageMaker::GetInstance()->StageStart();
	GameObject* btnObj = new GameObject();
	ColorButton* btn = new ColorButton();
	btnObj->AddComponent(btn);
	btnObj->InitializeSet();
	btn->SetUIPos({ -700,300, -1.0f });
	btn->SetUISize({ 200,100 });
	btn->SetText(TEXT("GameScene Load"));
	btn->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	btn->SetDefaultColor(RGB(255, 255, 255));
	btn->SetHoverColor(RGB(200, 200, 200));
	btn->SetDownColor(RGB(150, 150, 150));
	btn->SetTextSize(20);
	btn->SetEvent(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

	GameObject* btnObj2 = new GameObject();
	ColorButton* btn2 = new ColorButton();
	btnObj2->AddComponent(btn2);
	btnObj2->InitializeSet();
	btn2->SetUIPos({ -700,200, -1.0f });
	btn2->SetUISize({ 200,100 });
	btn2->SetText(TEXT("StartScene Load"));
	btn2->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	btn2->SetDefaultColor(RGB(255, 255, 255));
	btn2->SetHoverColor(RGB(200, 200, 200));
	btn2->SetDownColor(RGB(150, 150, 150));
	btn2->SetTextSize(20);
	btn2->SetEvent(bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
}

void GameScene::Init()
{
	m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG");
	SceneChanger::Create();
	StageMaker::Create();
}

void GameScene::Release()
{
	AnimationManager::ReleaseTexture(m_bg);
	SceneChanger::Destroy();
	StageMaker::Destroy();
}

void GameScene::Start()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	GameObject* btnObj = new GameObject();
	m_btn = new ColorButton();
	btnObj->AddComponent(m_btn);
	btnObj->InitializeSet();
	m_btn->SetUIPos({ -700,200,-1.0f });
	m_btn->SetUISize({ 200,100 });
	m_btn->SetText(TEXT("Start Game"));
	m_btn->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	m_btn->SetDefaultColor(RGB(255, 255, 255));
	m_btn->SetHoverColor(RGB(200, 200, 200));
	m_btn->SetDownColor(RGB(150, 150, 150));
	m_btn->SetTextSize(20);
	m_btn->SetEvent(bind(&GameScene::StartGame, this));
}

