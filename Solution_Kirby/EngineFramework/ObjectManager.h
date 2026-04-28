#pragma once
#include "GameObject.h"

class ObjectManager
{
private:
	static ObjectManager* m_Pthis;
	list<GameObject*>* m_objList;
	vector<GameObject*> m_pendingAddObjects;
	vector<GameObject*> m_pendingRemoveObjects;
	GameObject* m_selected;
	bool m_isFlushing = false;

	bool IsInObjectList(GameObject* obj);
	bool IsPendingAdd(GameObject* obj);
	bool IsPendingRemove(GameObject* obj);
	void QueueDestroyObject(GameObject* obj);
	void QueueDestroyObjectTree(GameObject* obj);
	bool IsSameOrChild(GameObject* root, GameObject* target);
	void ReleaseAndDeleteObject(GameObject* obj);
	bool IsGameObjectNameUsed(const string& name);
	string CreateUniqueGameObjectName(const string& baseName);
	GameObject* CreateEmptyGameObject();
	GameObject* CreateSpriteGameObject();
	GameObject* CreateUIImageGameObject();
	GameObject* CreateUIButtonGameObject();
	GameObject* CreateUITextGameObject();
	void DrawHierarchy();
	void DrawGameObjectDeleteControls(GameObject* obj);
	void DrawGameObjectInspector(GameObject* obj);
	bool DrawComponentInspector(Component* component);
	
public:
	static void Create();
	static ObjectManager* GetInstance();
	static void Destroy();

	void ProcessChildNode(GameObject* obj, int depth);
	void ImguiUpdate();
	void AddObject(GameObject* obj);
	bool UnregisterObject(GameObject* obj);
	void RegisterObject(GameObject* obj);
	bool DestroyObject(GameObject* obj);
	bool DestroyObject(string tag);
	GameObject* FindObject(string tag);
	bool FindObject(GameObject* obj);
	void Initialize();
	void Release();
	void Update();
	void FlushPendingObjects();
	void Clear();
	int Count();
	list<GameObject*>* GetObjList();
	std::string SerializeObjects();
	bool DeserializeObjects(const std::string& sceneJson);

	void OnLBtnDown();
	void OnLBtnUp();
	void OnRBtnDown();
	void OnRBtnUp();
};

