#pragma once

#include "../EngineFrameworkAPI.h"

class RenderManager;

class ENGINEFRAMEWORK_API EditorRenderFacade
{
public:
	static void DrawOverlay(RenderManager& renderManager);
};
