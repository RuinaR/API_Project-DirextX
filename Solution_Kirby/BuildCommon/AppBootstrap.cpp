#include "AppBootstrap.h"

#include "../EngineFrameworkDll/pch.h"

namespace
{
	class StartupScene : public Scene
	{
	public:
		explicit StartupScene(const std::string& sceneName)
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
}

bool BootstrapMainFrame(HINSTANCE hInstance, const AppLaunchDesc& launchDesc)
{
	MainFrame::Create(hInstance);
	WindowFrame::GetInstance()->SetRequestedSceneDataName(launchDesc.startupSceneName);

	Scene* startupScene = new StartupScene(launchDesc.startupSceneName);
	MainFrame::GetInstance()->Initialize(launchDesc.targetFps, startupScene, launchDesc.renderType);
	return true;
}

bool UpdateMainFrame()
{
	MainFrame* mainFrame = MainFrame::GetInstance();
	if (mainFrame == nullptr)
	{
		return false;
	}

	return mainFrame->Update();
}

void ShutdownMainFrame()
{
	if (MainFrame::GetInstance() != nullptr)
	{
		MainFrame::Destroy();
	}
}
