#pragma once

class GameObject;

namespace ActivePhysicsTestActionKeys
{
	static const char* const kToggleActiveActionKey = "ActivePhysicsTest.TogglePlayerActive";
	static const char* const kToggleTriggerActionKey = "ActivePhysicsTest.TogglePlayerTrigger";
}

void SetActivePhysicsTestPlayerObject(GameObject* playerObject);
void ExecuteActivePhysicsTestTogglePlayerActiveAction();
void ExecuteActivePhysicsTestTogglePlayerTriggerAction();
