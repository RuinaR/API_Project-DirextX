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
#include "UIActionRegistry.h"

static UIButton* CreateSceneButton(const D3DXVECTOR2* position, const wchar_t* text, std::function<void()> onClick, const std::string& actionKey = "", int orderInLayer = 10)
{
	if (!position)
	{
		return nullptr;
	}

	GameObject* buttonObj = new GameObject();
	UIButton* button = new UIButton();
	buttonObj->AddComponent(button);
	buttonObj->InitializeSet();
	D3DXVECTOR2 buttonSize(220.0f, 70.0f);
	button->SetPosition(position);
	button->SetSize(&buttonSize);
	button->SetUseTexture(false);
	button->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 215, 235, 255), D3DCOLOR_ARGB(255, 180, 205, 230));
	button->SetOrderInLayer(orderInLayer);
	button->SetActionKey(actionKey);
	button->SetOnClick(onClick);

	GameObject* labelObj = new GameObject();
	UILabel* label = new UILabel();
	labelObj->AddComponent(label);
	labelObj->InitializeSet();
	labelObj->SetParent(buttonObj);
	D3DXVECTOR2 labelPosition(position->x + 26.0f, position->y + 23.0f);
	D3DXVECTOR2 labelSize(190.0f, 28.0f);
	label->SetPosition(&labelPosition);
	label->SetSize(&labelSize);
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
	D3DXVECTOR2 gameSceneButtonPosition(40.0f, 40.0f);
	D3DXVECTOR2 startSceneButtonPosition(40.0f, 125.0f);
	CreateSceneButton(&gameSceneButtonPosition, L"GameScene Load", bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()), "ChangeGameScene");
	CreateSceneButton(&startSceneButtonPosition, L"StartScene Load", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()), "ChangeStartScene");
}

void GameScene::Init()
{
	m_bg = ResourceManager::GetInstance()->GetTexture("Bitmaps\\obj\\BG.bmp");
	SceneChanger::Create();
	UIActionRegistry::RegisterAction("ChangeGameScene", bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));
	UIActionRegistry::RegisterAction("ChangeStartScene", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
	StageMaker::Create();
}

void GameScene::Release()
{
	SceneChanger::Destroy();
	UIActionRegistry::UnregisterAction("ChangeGameScene");
	UIActionRegistry::UnregisterAction("ChangeStartScene");
	StageMaker::Destroy();
}

void GameScene::Start()
{
}

const char* GameScene::GetSceneName() const
{
	return "GameScene";
}

void GameScene::BuildInitialSceneObjects()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	D3DXVECTOR2 startGameButtonPosition(40.0f, 40.0f);
	m_btn = CreateSceneButton(&startGameButtonPosition, L"Start Game", bind(&GameScene::StartGame, this));
}

