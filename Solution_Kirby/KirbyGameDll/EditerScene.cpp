#include "pch.h"
#include "EditerScene.h"
#include "UIButton.h"
#include "UILabel.h"
#include "SceneChanger.h"
#include "DebugWindow.h"
#include "UIActionRegistry.h"

static UIButton* CreateEditorSceneButton(
	const D3DXVECTOR2* position,
	const wchar_t* text,
	std::function<void()> onClick,
	const std::string& actionKey = "",
	int orderInLayer = 10,
	const D3DXVECTOR2* size = nullptr,
	int fontSize = 20)
{
	if (!position)
	{
		return nullptr;
	}

	D3DXVECTOR2 defaultSize(220.0f, 70.0f);
	const D3DXVECTOR2* buttonSize = size ? size : &defaultSize;

	GameObject* buttonObj = new GameObject();
	UIButton* button = new UIButton();
	buttonObj->AddComponent(button);
	buttonObj->InitializeSet();
	button->SetPosition(position);
	button->SetSize(buttonSize);
	button->SetUseTexture(false);
	button->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 225, 240, 220), D3DCOLOR_ARGB(255, 190, 215, 185));
	button->SetOrderInLayer(orderInLayer);
	button->SetActionKey(actionKey);
	button->SetOnClick(onClick);

	GameObject* labelObj = new GameObject();
	UILabel* label = new UILabel();
	labelObj->AddComponent(label);
	labelObj->InitializeSet();
	labelObj->SetParent(buttonObj);
	const float labelPaddingX = buttonSize->x <= 170.0f ? 10.0f : 28.0f;
	const float labelPaddingY = buttonSize->y <= 50.0f ? 10.0f : 23.0f;
	D3DXVECTOR2 labelPosition(position->x + labelPaddingX, position->y + labelPaddingY);
	D3DXVECTOR2 labelSize(buttonSize->x - (labelPaddingX * 2.0f), buttonSize->y - (labelPaddingY * 2.0f));
	label->SetPosition(&labelPosition);
	label->SetSize(&labelSize);
	label->SetText(text);
	label->SetColor(D3DCOLOR_ARGB(255, 40, 40, 40));
	label->SetFontSize(fontSize);
	label->SetOrderInLayer(orderInLayer + 10);

	return button;
}

void EditerScene::StartEdit()
{
	string mapName = m_input->GetString();
	if (mapName.empty())
	{
		MessageBox(WindowFrame::GetInstance()->GetHWND(), TEXT("맵 이름을 입력 하세요"), TEXT("알림"), MB_OK);
		return;
	}
	m_btn->GetGameObject()->SetDestroy(true);
	m_input->GetGameObject()->SetDestroy(true);

	GameObject* newObj = new GameObject();
	m_edit = new Edit();
	newObj->AddComponent(m_edit);
	newObj->InitializeSet();
	m_edit->SetMap(mapName);

	D3DXVECTOR2 startButtonPosition(40.0f, 90.0f);
	D3DXVECTOR2 startButtonSize(150.0f, 42.0f);
	CreateEditorSceneButton(&startButtonPosition, L"StartScene Load", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()), "ChangeStartScene", 10, &startButtonSize, 14);
}

void EditerScene::Init()
{
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
	SceneChanger::Create();
	UIActionRegistry::RegisterAction("ChangeStartScene", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
}

void EditerScene::Release()
{
	SceneChanger::Destroy();
	UIActionRegistry::UnregisterAction("ChangeStartScene");
}

void EditerScene::Start()
{
}

const char* EditerScene::GetSceneName() const
{
	return "EditerScene";
}

void EditerScene::BuildInitialSceneObjects()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	D3DXVECTOR2 startEditButtonPosition(40.0f, 40.0f);
	m_btn = CreateEditorSceneButton(&startEditButtonPosition, L"Start Edit", bind(&EditerScene::StartEdit, this));
}
