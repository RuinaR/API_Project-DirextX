#include "pch.h"
#include "SceneChanger.h"
#include "Scene.h"
#include "GameScene.h"
#include "StartScene.h"
#include "EditerScene.h"
SceneChanger* SceneChanger::m_Pthis = nullptr;

void SceneChanger::Initialize()
{
}

void SceneChanger::Release()
{
}

void SceneChanger::Start()
{
}

void SceneChanger::Update()
{
}

SceneChanger* SceneChanger::GetInstance()
{
	return m_Pthis;
}

void SceneChanger::Create()
{
	if (m_Pthis == nullptr)
	{
		m_Pthis = new SceneChanger();
		GameObject* DW = new GameObject();
		DW->AddComponent(m_Pthis);
		DW->SetTag("SceneChanger");
		DW->InitializeSet();
	}
}

void SceneChanger::Destroy()
{
	if (m_Pthis != nullptr)
	{
		if (m_Pthis->m_gameObj != nullptr)
		{
			m_Pthis->m_gameObj->SetDestroy(true);
		}
		else
		{
			delete m_Pthis;
		}
		m_Pthis = nullptr;
	}
}

void SceneChanger::ChangeGameScene()
{
	WindowFrame::GetInstance()->SetScene(new GameScene());
}

void SceneChanger::ChangeStartScene()
{
	WindowFrame::GetInstance()->SetScene(new StartScene());
}

void SceneChanger::ChangeEditScene()
{
	WindowFrame::GetInstance()->SetScene(new EditerScene());
}
