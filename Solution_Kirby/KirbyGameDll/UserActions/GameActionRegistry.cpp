#include "pch.h"
#include "GameActionRegistry.h"

#include "Scripts/SceneChangeAction.h"
#include "UIActionRegistry.h"

void RegisterGameUIActions(RenderType type)
{
	(void)type;
	SceneChangeAction::RegisterActions();
}
