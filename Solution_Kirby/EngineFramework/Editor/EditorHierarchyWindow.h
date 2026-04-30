#pragma once

class GameObject;

class EditorHierarchyWindow
{
public:
	static void Draw();

private:
	static void DrawGameObjectNode(GameObject* obj, int depth, int& visibleIndex);
	static void ProcessChildNode(GameObject* obj, int depth);
	static void DrawGameObjectDeleteControls(GameObject* obj);
	static void DrawOpenScenePopup();
	static void DrawSaveSceneAsPopup();
	static void DrawSceneDirtyConfirmPopup();
};
