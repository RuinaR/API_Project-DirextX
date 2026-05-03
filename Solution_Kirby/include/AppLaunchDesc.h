#pragma once

#include <string>

#include "RenderType.h"

struct AppLaunchDesc
{
	RenderType renderType = RenderType::Game;
	std::string startupSceneName;
	bool requireExistingSceneData = false;
	bool createNewSceneIfMissing = false;
	int targetFps = 60;
};
