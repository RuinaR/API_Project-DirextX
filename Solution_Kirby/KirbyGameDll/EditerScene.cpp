#include "pch.h"
#include "EditerScene.h"
#include "UIButton.h"
#include "UILabel.h"
#include "SceneChanger.h"
#include "DebugWindow.h"

static UIButton* CreateEditorSceneButton(
	const D3DXVECTOR2& position,
	const wchar_t* text,
	std::function<void()> onClick,
	int orderInLayer = 10,
	const D3DXVECTOR2& size = D3DXVECTOR2(220.0f, 70.0f),
	int fontSize = 20)
{
	GameObject* buttonObj = new GameObject();
	UIButton* button = new UIButton();
	buttonObj->AddComponent(button);
	buttonObj->InitializeSet();
	button->SetPosition(position);
	button->SetSize(size);
	button->SetUseTexture(false);
	button->SetStateColors(D3DCOLOR_ARGB(255, 245, 245, 245), D3DCOLOR_ARGB(255, 225, 240, 220), D3DCOLOR_ARGB(255, 190, 215, 185));
	button->SetOrderInLayer(orderInLayer);
	button->SetOnClick(onClick);

	GameObject* labelObj = new GameObject();
	UILabel* label = new UILabel();
	labelObj->AddComponent(label);
	labelObj->InitializeSet();
	labelObj->SetParent(buttonObj);
	const float labelPaddingX = size.x <= 170.0f ? 10.0f : 28.0f;
	const float labelPaddingY = size.y <= 50.0f ? 10.0f : 23.0f;
	label->SetPosition(D3DXVECTOR2(position.x + labelPaddingX, position.y + labelPaddingY));
	label->SetSize(D3DXVECTOR2(size.x - (labelPaddingX * 2.0f), size.y - (labelPaddingY * 2.0f)));
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

	CreateEditorSceneButton(D3DXVECTOR2(40.0f, 90.0f), L"StartScene Load", bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()), 10, D3DXVECTOR2(150.0f, 42.0f), 14);
}

void EditerScene::Init()
{
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
	SceneChanger::Create();
}

void EditerScene::Release()
{
	SceneChanger::Destroy();
}

void EditerScene::Start()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	m_btn = CreateEditorSceneButton(D3DXVECTOR2(40.0f, 40.0f), L"Start Edit", bind(&EditerScene::StartEdit, this));
}
