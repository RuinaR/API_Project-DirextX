#include "pch.h"
#include "EditerScene.h"
#include "ColorButton.h"
#include "SceneChanger.h"
#include "DebugWindow.h"
void EditerScene::StartEdit()
{
	string mapName = m_input->GetString();
	if (mapName.empty())
	{
		MessageBox(WindowFrame::GetInstance()->GetHWND(), TEXT("�� �̸��� �Է� �ϼ���"), TEXT("�˸�"), MB_OK);
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
	ColorButton* btn = new ColorButton();
	btnObj->AddComponent(btn);
	btnObj->SetOrderInLayer(10);
	btnObj->InitializeSet();
	btn->SetUIPos({ 0,10 });
	btn->SetUISize({ 190,50 });
	btn->SetText(TEXT("StartScene Load"));
	btn->SetTextColor(RGB(255, 0, 255));
	btn->SetDefaultColor(RGB(255, 255, 255));
	btn->SetHoverColor(RGB(200, 200, 200));
	btn->SetDownColor(RGB(150, 150, 150));
	btn->SetTextSize(20);
	btn->SetEvent(bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
}
void EditerScene::Init()
{
	m_bg = AnimationManager::LoadHBitmap("Bitmaps\\obj\\BG");
	SceneChanger::Create();
	if (DEBUGMODE)
	{
		DebugWindow::Create();
		DebugWindow::GetInstance()->SetDWPos({ 0,400 });
	}
}

void EditerScene::Release()
{
	AnimationManager::ReleaseHBitmap(m_bg);
	SceneChanger::Destroy();
	if (DEBUGMODE)
		DebugWindow::Destroy();
}

void EditerScene::Start()
{
	WindowFrame::GetInstance()->GetBuffer()->SetBG(m_bg);

	GameObject* obj = new GameObject();
	m_input = new InputString();
	obj->AddComponent(m_input);
	obj->SetOrderInLayer(10);
	obj->InitializeSet();

	GameObject* btnObj = new GameObject();
	m_btn = new ColorButton();
	btnObj->AddComponent(m_btn);
	btnObj->SetOrderInLayer(10);
	btnObj->InitializeSet();
	m_btn->SetUIPos({ 0,200 });
	m_btn->SetUISize({ 190,50 });
	m_btn->SetText(TEXT("Start Edit"));
	m_btn->SetTextColor(RGB(255, 0, 255));
	m_btn->SetDefaultColor(RGB(255, 255, 255));
	m_btn->SetHoverColor(RGB(200, 200, 200));
	m_btn->SetDownColor(RGB(150, 150, 150));
	m_btn->SetTextSize(20);
	m_btn->SetEvent(bind(&EditerScene::StartEdit, this));
}
