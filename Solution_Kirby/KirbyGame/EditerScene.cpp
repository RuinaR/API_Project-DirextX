#include "pch.h"
#include "EditerScene.h"
#include "Button.h"
#include "SceneChanger.h"
#include "DebugWindow.h"
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

	GameObject* btnObj = new GameObject();
	Button* btn = new Button();
	btnObj->AddComponent(btn);
	btnObj->InitializeSet();
	btn->SetUIPos({ 0,400,-1.0f });
	btn->SetUISize({ 190,50 });
	btn->SetText("StartScene Load");
	btn->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	btn->SetDefaultColor(RGB(255, 255, 255));
	btn->SetHoverColor(RGB(200, 200, 200));
	btn->SetDownColor(RGB(150, 150, 150));
	btn->SetTextSize(20);
	btn->SetEvent(bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
}
void EditerScene::Init()
{
	m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG.bmp");
	SceneChanger::Create();
}

void EditerScene::Release()
{
	AnimationManager::ReleaseTexture(m_bg);
	SceneChanger::Destroy();
}

void EditerScene::Start()
{
	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->InitializeSet();

	GameObject* btnObj = new GameObject();
	m_btn = new Button();
	btnObj->AddComponent(m_btn);
	btnObj->InitializeSet();
	m_btn->SetUIPos({ 0,200,-1.0f });
	m_btn->SetUISize({ 190,50 });
	m_btn->SetText("Start Edit");
	m_btn->SetTextColor(D3DCOLOR_XRGB(255, 0, 255));
	m_btn->SetDefaultColor(RGB(255, 255, 255));
	m_btn->SetHoverColor(RGB(200, 200, 200));
	m_btn->SetDownColor(RGB(150, 150, 150));
	m_btn->SetTextSize(20);
	m_btn->SetEvent(bind(&EditerScene::StartEdit, this));
}
