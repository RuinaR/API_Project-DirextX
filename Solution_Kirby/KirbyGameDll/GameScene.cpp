#include "pch.h"
#include "GameScene.h"
#include "Component.h"
#include "GameObject.h"
#include "Player.h"
#include "ChangeObject.h"
#include "StageMaker.h"
#include "UIButton.h"
#include "UILabel.h"
#include "DebugWindow.h"
#include "SceneChanger.h"

static UIButton* CreateSceneButton(const D3DXVECTOR2& position, const wchar_t* text, std::function<void()> onClick, int orderInLayer = 10)
{
	GameObject* buttonObj = new GameObject();
	UIButton* button = new UIButton();
	buttonObj->AddComponent(button);
	buttonObj->InitializeSet();
	button->SetPosition(position);
	button->SetSize(D3DXVECTOR2(220.0f, 70.0f));
	button->SetUseTexture(false);
	button->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 215, 235, 255), D3DCOLOR_ARGB(255, 180, 205, 230));
	button->SetOrderInLayer(orderInLayer);
	button->SetOnClick(onClick);

	GameObject* labelObj = new GameObject();
	UILabel* label = new UILabel();
	labelObj->AddComponent(label);
	labelObj->InitializeSet();
	labelObj->SetParent(buttonObj);
	label->SetPosition(D3DXVECTOR2(position.x + 26.0f, position.y + 23.0f));
	label->SetSize(D3DXVECTOR2(190.0f, 28.0f));
	label->SetText(text);
	label->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	label->SetFontSize(20);
	label->SetOrderInLayer(orderInLayer + 10);

	return button;
}

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
	CreateSceneButton(D3DXVECTOR2(40.0f, 40.0f), L"GameScene Load", bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));
	CreateSceneButton(D3DXVECTOR2(40.0f, 125.0f), L"StartScene Load", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
}

void GameScene::Init()
{
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
	SceneChanger::Create();
	StageMaker::Create();
}

void GameScene::Release()
{
	SceneChanger::Destroy();
	StageMaker::Destroy();
}

void GameScene::Start()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	m_btn = CreateSceneButton(D3DXVECTOR2(40.0f, 40.0f), L"Start Game", bind(&GameScene::StartGame, this));
}

