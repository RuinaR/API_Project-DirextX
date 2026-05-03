#include "AppLaunchDesc.h"

#include "../EngineFrameworkDll/pch.h"
#include "../EngineFrameworkDll/Scene/BuildSettingsManager.h"

namespace
{
	const char* kDefaultEditorStartupSceneName = "NewScene";
	const char* kDefaultGameStartupSceneName = "StartScene";
}

AppLaunchDesc BuildEditorLaunchDesc()
{
	AppLaunchDesc desc;
	desc.renderType = RenderType::Edit;
	desc.requireExistingSceneData = false;
	desc.createNewSceneIfMissing = true;
	desc.targetFps = TARGETFPS;

	if (!BuildSettingsManager::TryGetStartupSceneName(desc.startupSceneName))
	{
		desc.startupSceneName = kDefaultEditorStartupSceneName;
	}

	return desc;
}

AppLaunchDesc BuildGameLaunchDesc()
{
	AppLaunchDesc desc;
	desc.renderType = RenderType::Game;
	desc.requireExistingSceneData = true;
	desc.createNewSceneIfMissing = false;
	desc.targetFps = TARGETFPS;

	if (!BuildSettingsManager::TryGetStartupSceneName(desc.startupSceneName))
	{
		desc.startupSceneName = kDefaultGameStartupSceneName;
	}

	return desc;
}
