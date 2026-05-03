#include "pch.h"
#include "GameComponentRegistry.h"

#include "ComponentFactory.h"
#include "Scripts/CookieRunGameManagerComponent.h"
#include "Scripts/CookieRunItemComponent.h"
#include "Scripts/CookieRunObstacleComponent.h"
#include "Scripts/CookieRunPlayerControllerComponent.h"
#include "Scripts/CookieRunResultComponent.h"
#include "Scripts/CookieRunRotateYComponent.h"
#include "Scripts/CookieRunScrollingPoolComponent.h"
#include "Scripts/LoopObjectPoolComponent.h"
#include "Scripts/SpawnObjectPoolComponent.h"
#include "UserComponentRegistration.h"

void RegisterGameComponents(ComponentFactory& factory)
{
	REGISTER_GAME_COMPONENT(factory, CookieRunGameManagerComponent, "CookieRun Game Manager", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunPlayerControllerComponent, "CookieRun Player Controller", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, LoopObjectPoolComponent, "Loop Object Pool", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, SpawnObjectPoolComponent, "Spawn Object Pool", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunScrollingPoolComponent, "CookieRun Scrolling Pool", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunItemComponent, "CookieRun Item", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunObstacleComponent, "CookieRun Obstacle", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunResultComponent, "CookieRun Result", "CookieRun");
	REGISTER_GAME_COMPONENT(factory, CookieRunRotateYComponent, "CookieRun Rotate Y", "CookieRun");
}
