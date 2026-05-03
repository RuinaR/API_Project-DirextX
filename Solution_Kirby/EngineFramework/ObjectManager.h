#pragma once
#include "GameObject.h"
#include "Math/Ray.h"
#include "Physics2D/Physics2D.h"
#include <functional>
#include <unordered_map>

class ObjectManager
{
private:
	static ObjectManager* m_Pthis;
	list<GameObject*>* m_objList;
	vector<GameObject*> m_pendingAddObjects;
	vector<GameObject*> m_pendingRemoveObjects;
	bool m_isFlushing = false;
	int m_nextRuntimeObjectId = 1;
	Ray m_lastMouseRay;
	RaycastHit2D m_lastMouseRaycastHit;
	bool m_hasLastMouseRay = false;
	bool m_hasLastMouseRaycastHit = false;
	GameObject* m_currentMouseHoverObject = nullptr;
	bool m_mouseInteractionBlocked = false;
	bool m_isClearInProgress = false;
	vector<std::function<void(GameObject*)>> m_objectDestroyRequestedCallbacks;
	vector<std::function<void(GameObject*)>> m_objectReleasedCallbacks;
	vector<std::function<void()>> m_objectsClearingCallbacks;
	vector<std::function<void()>> m_objectsClearedCallbacks;

	bool IsInObjectList(GameObject* obj);
	bool IsPendingAdd(GameObject* obj);
	bool IsPendingRemove(GameObject* obj);
	void AssignRuntimeObjectId(GameObject* obj);
	void ResolveComponentReferences(const std::unordered_map<int, GameObject*>& objectMap);
	// 삭제는 즉시 하지 않고 pending remove 예약으로 처리한다.
	// 실제 Release/delete는 FlushPendingObjects()에서 일괄 수행한다.
	void QueueDestroyObject(GameObject* obj);
	void QueueDestroyObjectTree(GameObject* obj);
	void ReleaseAndDeleteObject(GameObject* obj);
	GameObject* RaycastMouseToGameObject();
	void NotifyObjectDestroyRequested(GameObject* obj);
	void NotifyObjectReleased(GameObject* obj);
	void NotifyObjectsClearing();
	void NotifyObjectsCleared();
	
public:
	static void Create();
	static ObjectManager* GetInstance();
	static void Destroy();

	void AddObject(GameObject* obj);
	bool UnregisterObject(GameObject* obj);
	void RegisterObject(GameObject* obj);
	bool DestroyObject(GameObject* obj);
	// Legacy tag-based delete wrapper. Prefer DestroyFirstObjectByTag/DestroyObjectsByTag in new code.
	bool DestroyObject(string tag);
	bool DestroyObjectByName(const string& name);
	bool DestroyFirstObjectByTag(const string& tag);
	int DestroyObjectsByTag(const string& tag);
	// Legacy tag-based search wrapper. Prefer FindObjectByName/FindFirstObjectByTag in new code.
	GameObject* FindObject(string tag);
	GameObject* FindObjectByName(const string& name);
	GameObject* FindFirstObjectByTag(const string& tag);
	vector<GameObject*> FindObjectsByTag(const string& tag);
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
	void RegisterOnObjectDestroyRequested(const std::function<void(GameObject*)>& callback);
	void RegisterOnObjectReleased(const std::function<void(GameObject*)>& callback);
	void RegisterOnObjectsClearing(const std::function<void()>& callback);
	void RegisterOnObjectsCleared(const std::function<void()>& callback);
	void SetMouseInteractionBlocked(bool blocked);
	bool IsMouseInteractionBlocked() const;
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

