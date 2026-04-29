#pragma once

class Component;
class GameObject;

class EditorInspectorWindow
{
public:
	static void Draw();

private:
	static void DrawGameObjectInspector(GameObject* obj);
	static bool DrawComponentInspector(Component* component);
};
