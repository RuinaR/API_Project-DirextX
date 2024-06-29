#pragma once
#include "GameObject.h"

class ObjectManager
{
private:
	static ObjectManager* m_Pthis;
	list<GameObject*>* m_objList = nullptr;
	list<GameObject*>::iterator m_selected;
	void ImguiUpdate();
public:
	static void Create();
	static ObjectManager* GetInstance();
	static void Destroy();

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
	void Clear();
	int Count();
	list<GameObject*>* GetObjList();

	void OnLBtnDown();
	void OnLBtnUp();
	void OnRBtnDown();
	void OnRBtnUp();
};

