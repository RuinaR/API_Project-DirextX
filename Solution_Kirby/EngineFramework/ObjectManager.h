#pragma once
#include "GameObject.h"
#include "Math/Ray.h"
#include "Physics2D/Physics2D.h"
#include <unordered_map>

class ObjectManager
{
private:
	static ObjectManager* m_Pthis;
	list<GameObject*>* m_objList;
	vector<GameObject*> m_pendingAddObjects;
	vector<GameObject*> m_pendingRemoveObjects;
	GameObject* m_selected;
	bool m_isFlushing = false;
	int m_nextRuntimeObjectId = 1;
	Ray m_lastMouseRay;
	RaycastHit2D m_lastMouseRaycastHit;
	bool m_hasLastMouseRay = false;
	bool m_hasLastMouseRaycastHit = false;
	GameObject* m_currentMouseHoverObject = nullptr;

	bool IsInObjectList(GameObject* obj);
	bool IsPendingAdd(GameObject* obj);
	bool IsPendingRemove(GameObject* obj);
	void AssignRuntimeObjectId(GameObject* obj);
	void ResolveComponentReferences(const std::unordered_map<int, GameObject*>& objectMap);
	// 삭제는 즉시 delete가 아니라 pending remove 예약으로 처리한다.
	// 실제 Release/delete는 FlushPendingObjects()에서 일괄 수행한다.
	void QueueDestroyObject(GameObject* obj);
	void QueueDestroyObjectTree(GameObject* obj);
	bool IsSameOrChild(GameObject* root, GameObject* target);
	void ReleaseAndDeleteObject(GameObject* obj);
	GameObject* RaycastMouseToGameObject();
	
public:
	static void Create();
	static ObjectManager* GetInstance();
	static void Destroy();

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
	void UpdateMouseInteraction();
	void FlushPendingObjects();
	void Clear();
	int Count();
	list<GameObject*>* GetObjList();
	bool HasGameObjectName(const string& name);
	GameObject* GetSelectedObject();
	void SetSelectedObject(GameObject* obj);
	void ClearSelectedObject();
	std::string SerializeObjects();
	std::string SerializeObjects(int sceneVersion);
	bool DeserializeObjects(const std::string& sceneJson);
	bool DeserializeObjects(const std::string& sceneJson, int sceneVersion);

	void OnLBtnDown();
	void OnLBtnUp();
	void OnRBtnDown();
	void OnRBtnUp();
	bool GetLastMouseRaycastInfo(Ray& outRay, RaycastHit2D& outHit, bool& outHasHit) const;
	GameObject* GetCurrentMouseHoverObject() const { return m_currentMouseHoverObject; }
};

