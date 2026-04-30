#pragma once

class Component;
class GameObject;

class EditorInspectorWindow
{
public:
	static void Draw();
	static bool DrawGameObjectReferenceField(const char* label, GameObject*& ref, int& refObjectId);

private:
	static void DrawGameObjectInspector(GameObject* obj);
	static bool DrawComponentInspector(Component* component);
};
