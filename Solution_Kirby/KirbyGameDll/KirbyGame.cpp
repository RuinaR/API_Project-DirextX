#include "pch.h"
#include "KirbyGame.h"
#include "../include/PluginManager.h"

#include "BuildSettingsManager.h"
#include "SceneDataManager.h"
#include "ComponentFactory.h"
#include "UserComponents/GameComponentRegistry.h"
#include "UIActionRegistry.h"
#include "UserActions/GameActionRegistry.h"

namespace
{
	const char* kDefaultGameStartupSceneName = "StartScene";
	const char* kDefaultEditStartupSceneName = "NewScene";

	std::wstring ToWideString(const std::string& text)
	{
		if (text.empty())
		{
			return std::wstring();
		}

		const int requiredLength = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
		if (requiredLength <= 0)
		{
			return std::wstring(text.begin(), text.end());
		}

		std::wstring wideText(static_cast<size_t>(requiredLength) - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], requiredLength);
		return wideText;
	}

	class RuntimeScene : public Scene
	{
	public:
		explicit RuntimeScene(const std::string& sceneName)
			: m_sceneName(sceneName)
		{
		}

		void Init() override {}
		void Release() override {}
		void Start() override {}
		const char* GetSceneName() const override { return m_sceneName.c_str(); }
		void BuildInitialSceneObjects() override {}

	private:
		std::string m_sceneName;
	};

	std::string GetStartupSceneName(RenderType type, bool* outUsedBuildSettings = nullptr)
	{
		std::string startupSceneName;
		if (BuildSettingsManager::TryGetStartupSceneName(startupSceneName))
		{
			if (outUsedBuildSettings != nullptr)
			{
				*outUsedBuildSettings = true;
			}
			return startupSceneName;
		}

		if (outUsedBuildSettings != nullptr)
		{
			*outUsedBuildSettings = false;
		}

		if (type == RenderType::Edit)
		{
			return kDefaultEditStartupSceneName;
		}

		return kDefaultGameStartupSceneName;
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
	RegisterGameUIActions(type);
	bool usedBuildSettingsStartup = false;
	const std::string startupSceneName = GetStartupSceneName(type, &usedBuildSettingsStartup);
	const bool startupSceneExists = SceneDataManager::Exists(startupSceneName);

	if (type == RenderType::Game && !startupSceneExists)
	{
		const std::wstring message = std::wstring(L"시작 SceneData 파일을 찾지 못했습니다: ") + ToWideString(startupSceneName);
		MessageBoxW(nullptr, message.c_str(), L"SceneData Load Error", MB_OK | MB_ICONERROR);
		return false;
	}

	MainFrame::Create(hInst);
	WindowFrame::GetInstance()->SetRequestedSceneDataName(startupSceneName);
	Scene* startupScene = new RuntimeScene(startupSceneName);
	MainFrame::GetInstance()->Initialize(TARGETFPS, startupScene, type);

	if (type == RenderType::Edit && (!usedBuildSettingsStartup || !startupSceneExists))
	{
		SceneDataManager::CreateNewScene(startupSceneName);
	}
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
	UIActionRegistry::Clear();
	MainFrame::Destroy();
}


PLUGINDECL IPlugin* CreatePlugin(PluginManager& mgr)
{
	return new KirbyGame(mgr);
}
