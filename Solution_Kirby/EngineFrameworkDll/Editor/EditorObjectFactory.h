#pragma once

#include <string>

class Component;
class GameObject;

class EditorObjectFactory
{
public:
	static std::string CreateUniqueGameObjectName(const std::string& baseName);
	static GameObject* CreateEmptyGameObject();
	static GameObject* CreateSpriteGameObject();
	static GameObject* CreateUIImageGameObject();
	static GameObject* CreateUIButtonGameObject();
	static GameObject* CreateUITextGameObject();
	static void ApplyDefaultComponentValues(GameObject* obj, Component* component);
};
