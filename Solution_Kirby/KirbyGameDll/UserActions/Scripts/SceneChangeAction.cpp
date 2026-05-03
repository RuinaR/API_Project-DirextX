#include "pch.h"
#include "SceneChangeAction.h"

#include "DebugLog.h"
#include "MainFrame.h"
#include "UIActionRegistry.h"
#include "WindowFrame.h"

namespace
{
	void ApplySceneChange(const std::string& targetSceneName)
	{
		WindowFrame* windowFrame = WindowFrame::GetInstance();
		if (windowFrame == nullptr || targetSceneName.empty())
		{
			return;
		}

		if (MainFrame* mainFrame = MainFrame::GetInstance())
		{
			mainFrame->SetTimeScale(1.0f);
		}

		windowFrame->OpenSceneByName(targetSceneName);
	}
}

void SceneChangeAction::RegisterActions()
{
	UIActionRegistry::RegisterAction(kStartToGameActionKey, []()
		{
			DebugLog::Log("StartScene -> GameScene");
			RequestSceneChange("GameScene");
		});

	UIActionRegistry::RegisterAction(kResultToStartActionKey, []()
		{
			DebugLog::Log("ResultScene -> StartScene");
			RequestSceneChange("StartScene");
		});
}

void SceneChangeAction::RequestSceneChange(const std::string& targetSceneName)
{
	MainFrame* mainFrame = MainFrame::GetInstance();
	if (mainFrame == nullptr || targetSceneName.empty())
	{
		return;
	}

	mainFrame->QueueDeferredAction([targetSceneName]()
		{
			ApplySceneChange(targetSceneName);
		});
}
