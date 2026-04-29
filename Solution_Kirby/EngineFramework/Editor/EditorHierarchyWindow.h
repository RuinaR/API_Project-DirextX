#pragma once

class GameObject;

class EditorHierarchyWindow
{
public:
	static void Draw();

private:
	static void ProcessChildNode(GameObject* obj, int depth);
	static void DrawGameObjectDeleteControls(GameObject* obj);
};
