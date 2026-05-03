#pragma once

class Component;
class GameObject;

class ENGINEFRAMEWORK_API EditorInspectorWindow
{
public:
	static void Draw();
	static bool DrawGameObjectReferenceField(const char* label, GameObject*& ref, int& refObjectId);
	static bool DrawComponentReferenceField(const char* label, Component*& ref, const char* expectedType = nullptr);

private:
	static void DrawGameObjectInspector(GameObject* obj);
	static bool DrawComponentInspector(Component* component, bool* outInspectorChanged = nullptr);
};
