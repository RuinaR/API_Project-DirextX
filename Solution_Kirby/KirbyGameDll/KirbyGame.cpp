#include "pch.h"
#include "KirbyGame.h"
#include "../include/PluginManager.h"

#include "StartScene.h"
#include "GameScene.h"
#include "EditerScene.h"
#include "SceneDataManager.h"
#include "ComponentFactory.h"
#include "GameComponentRegistry.h"

namespace
{
	const char* GetStartupSceneName()
	{
		// 나중에 빌드 설정값으로 교체하기 쉽게 시작 씬 이름을 분리한다.
		return "StartScene";
	}

	Scene* CreateSceneByName(const std::string& sceneName)
	{
		if (sceneName == "GameScene")
		{
			return new GameScene();
		}
		if (sceneName == "EditerScene")
		{
			return new EditerScene();
		}
		return new StartScene();
	}
}


KirbyGame::KirbyGame(PluginManager& p_mgr) :
	m_hManager(p_mgr),
	m_strName("KirbyGame")
{

}

bool KirbyGame::Initialize(HINSTANCE hInst, RenderType type)
{
	RegisterEngineComponents();
	RegisterGameComponents(ComponentFactory::GetInstance());
	MainFrame::Create(hInst);
	const std::string startupSceneName = GetStartupSceneName();
	Scene* startupScene = CreateSceneByName(startupSceneName);
	MainFrame::GetInstance()->Initialize(TARGETFPS, startupScene, type);
	//MainFrame::GetInstance()->Set();
	return true;
}

bool KirbyGame::Shutdown()
{
	return false;
}

void KirbyGame::About(HWND hParent)
{

}

double KirbyGame::Execute(double a, double b)
{
	return 0.0;
}

const std::string KirbyGame::GetName()
{
	return m_strName;
}

void KirbyGame::AddObject(GameObject* p_obj)
{

}

void KirbyGame::RemoveObject(GameObject* p_obj)
{
}

bool KirbyGame::AllUpdate()
{
	return MainFrame::GetInstance()->Update();
}

void KirbyGame::AllStart()
{

}

void KirbyGame::AllRelease()
{
	MainFrame::Destroy();
}


PLUGINDECL IPlugin* CreatePlugin(PluginManager& mgr)
{
	return new KirbyGame(mgr);
}
