#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "SceneJsonUtility.h"
#include "ComponentFactory.h"
#include "Input/Mouse.h"
#include "Physics2D/Physics2D.h"

ObjectManager* ObjectManager::m_Pthis = nullptr;

namespace
{
	GameObject* SanitizeMouseEventTarget(GameObject* object)
	{
		if (object == nullptr)
		{
			return nullptr;
		}

		if (object->GetDestroy() || !object->GetActive())
		{
			return nullptr;
		}

		return object;
	}
}

bool ObjectManager::IsInObjectList(GameObject* obj)
{
	if (obj == nullptr || m_objList == nullptr)
		return false;

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

bool ObjectManager::IsPendingAdd(GameObject* obj)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

bool ObjectManager::IsPendingRemove(GameObject* obj)
{
	for (vector<GameObject*>::iterator itr = m_pendingRemoveObjects.begin(); itr != m_pendingRemoveObjects.end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

GameObject* ObjectManager::RaycastMouseToGameObject()
{
	Mouse* mouse = Mouse::GetInstance();
	if (mouse == nullptr)
	{
		m_hasLastMouseRay = false;
		m_hasLastMouseRaycastHit = false;
		m_lastMouseRaycastHit = RaycastHit2D();
		return nullptr;
	}

	m_lastMouseRay = mouse->ScreenPointToRay();
	m_hasLastMouseRay = true;
	m_hasLastMouseRaycastHit = Physics2D::Raycast(m_lastMouseRay, m_lastMouseRaycastHit, FLT_MAX, true);
	if (!m_hasLastMouseRaycastHit)
	{
		m_lastMouseRaycastHit = RaycastHit2D();
		return nullptr;
	}

	if (m_lastMouseRaycastHit.collider == nullptr ||
		m_lastMouseRaycastHit.gameObject == nullptr ||
		m_lastMouseRaycastHit.gameObject->GetDestroy() ||
		!m_lastMouseRaycastHit.gameObject->GetActive())
	{
		m_hasLastMouseRaycastHit = false;
		m_lastMouseRaycastHit = RaycastHit2D();
		return nullptr;
	}

	return m_lastMouseRaycastHit.gameObject;
}

void ObjectManager::AssignRuntimeObjectId(GameObject* obj)
{
	if (obj == nullptr)
		return;

	if (obj->GetId() >= 0)
	{
		if (obj->GetId() >= m_nextRuntimeObjectId)
		{
			m_nextRuntimeObjectId = obj->GetId() + 1;
		}
		return;
	}

	obj->SetId(m_nextRuntimeObjectId);
	m_nextRuntimeObjectId++;
}

void ObjectManager::ResolveComponentReferences(const std::unordered_map<int, GameObject*>& objectMap)
{
	for (list<GameObject*>::iterator objItr = m_objList->begin(); objItr != m_objList->end(); objItr++)
	{
		GameObject* obj = *objItr;
		if (obj == nullptr || obj->GetDestroy())
			continue;

		vector<Component*>* components = obj->GetComponentVec();
		if (components == nullptr)
			continue;

		for (vector<Component*>::iterator componentItr = components->begin(); componentItr != components->end(); componentItr++)
		{
			Component* component = *componentItr;
			if (component == nullptr)
				continue;

			component->ResolveReferences(objectMap);
		}
	}
}

void ObjectManager::NotifyObjectDestroyRequested(GameObject* obj)
{
	for (vector<std::function<void(GameObject*)>>::iterator itr = m_objectDestroyRequestedCallbacks.begin();
		itr != m_objectDestroyRequestedCallbacks.end();
		itr++)
	{
		if (*itr)
		{
			(*itr)(obj);
		}
	}
}

void ObjectManager::NotifyObjectReleased(GameObject* obj)
{
	for (vector<std::function<void(GameObject*)>>::iterator itr = m_objectReleasedCallbacks.begin();
		itr != m_objectReleasedCallbacks.end();
		itr++)
	{
		if (*itr)
		{
			(*itr)(obj);
		}
	}
}

void ObjectManager::NotifyObjectsClearing()
{
	for (vector<std::function<void()>>::iterator itr = m_objectsClearingCallbacks.begin();
		itr != m_objectsClearingCallbacks.end();
		itr++)
	{
		if (*itr)
		{
			(*itr)();
		}
	}
}

void ObjectManager::NotifyObjectsCleared()
{
	for (vector<std::function<void()>>::iterator itr = m_objectsClearedCallbacks.begin();
		itr != m_objectsClearedCallbacks.end();
		itr++)
	{
		if (*itr)
		{
			(*itr)();
		}
	}
}

void ObjectManager::QueueDestroyObject(GameObject* obj)
{
	if (obj == nullptr || IsPendingRemove(obj))
		return;

	obj->SetDestroy(true);
	m_pendingRemoveObjects.push_back(obj);
}

void ObjectManager::QueueDestroyObjectTree(GameObject* obj)
{
	if (obj == nullptr)
		return;

	vector<GameObject*> children;
	vector<GameObject*>* childList = obj->GetChild();
	if (childList != nullptr)
	{
		for (vector<GameObject*>::iterator itr = childList->begin(); itr != childList->end(); itr++)
		{
			if ((*itr) != nullptr)
			{
				children.push_back(*itr);
			}
		}
	}

	for (vector<GameObject*>::iterator itr = children.begin(); itr != children.end(); itr++)
	{
		QueueDestroyObjectTree(*itr);
	}

	QueueDestroyObject(obj);
}

void ObjectManager::ReleaseAndDeleteObject(GameObject* obj)
{
	if (obj == nullptr)
		return;

	NotifyObjectReleased(obj);
	if (m_currentMouseHoverObject == obj)
		m_currentMouseHoverObject = nullptr;

	obj->Release();
	delete obj;
}

bool ObjectManager::HasGameObjectName(const string& name)
{
	if (m_objList != nullptr)
	{
		for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
		{
			if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetName() == name)
			{
				return true;
			}
		}
	}

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetName() == name)
		{
			return true;
		}
	}

	return false;
}

void ObjectManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new ObjectManager();
	}
}

ObjectManager* ObjectManager::GetInstance()
{
	return m_Pthis;
}

void ObjectManager::Destroy()
{
	if (m_Pthis)
	{
		for (vector<GameObject*>::iterator itr = m_Pthis->m_pendingAddObjects.begin(); itr != m_Pthis->m_pendingAddObjects.end(); itr++)
		{
			m_Pthis->ReleaseAndDeleteObject(*itr);
		}
		m_Pthis->m_pendingAddObjects.clear();

		for (vector<GameObject*>::iterator itr = m_Pthis->m_pendingRemoveObjects.begin(); itr != m_Pthis->m_pendingRemoveObjects.end(); itr++)
		{
			GameObject* obj = *itr;
			for (list<GameObject*>::iterator listItr = m_Pthis->m_objList->begin(); listItr != m_Pthis->m_objList->end();)
			{
				if ((*listItr) == obj)
				{
					listItr = m_Pthis->m_objList->erase(listItr);
					break;
				}
				else
				{
					listItr++;
				}
			}
			m_Pthis->ReleaseAndDeleteObject(obj);
		}
		m_Pthis->m_pendingRemoveObjects.clear();

		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			m_Pthis->ReleaseAndDeleteObject(*itr);
			(*itr) = nullptr;
		}
		delete m_Pthis->m_objList;
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void ObjectManager::AddObject(GameObject* obj)
{
	if (obj == nullptr || IsInObjectList(obj) || IsPendingAdd(obj) || IsPendingRemove(obj))
		return;

	AssignRuntimeObjectId(obj);
	m_pendingAddObjects.push_back(obj);
}

bool ObjectManager::UnregisterObject(GameObject* obj)
{
	if (obj == nullptr)
		return false;

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) == obj)
		{
			m_pendingAddObjects.erase(itr);
			return true;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
		{
			m_objList->erase(itr);
			return true;
		}
	}
	return false;
}

void ObjectManager::RegisterObject(GameObject* obj)
{
	AddObject(obj);
}

bool ObjectManager::DestroyObject(GameObject* obj)
{
	if (obj == nullptr || IsPendingRemove(obj))
		return false;

	if (!IsInObjectList(obj) && !IsPendingAdd(obj))
		return false;

	NotifyObjectDestroyRequested(obj);
	QueueDestroyObjectTree(obj);
	return true;
}

bool ObjectManager::DestroyObject(string tag)
{
	return DestroyFirstObjectByTag(tag);
}

bool ObjectManager::DestroyObjectByName(const string& name)
{
	GameObject* target = FindObjectByName(name);
	if (target == nullptr)
	{
		return false;
	}

	return DestroyObject(target);
}

bool ObjectManager::DestroyFirstObjectByTag(const string& tag)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			return DestroyObject(*itr);
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			return DestroyObject(*itr);
		}
	}

	return false;
}

int ObjectManager::DestroyObjectsByTag(const string& tag)
{
	int destroyRequestedCount = 0;

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			if (DestroyObject(*itr))
			{
				destroyRequestedCount++;
			}
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			if (DestroyObject(*itr))
			{
				destroyRequestedCount++;
			}
		}
	}

	return destroyRequestedCount;
}

GameObject* ObjectManager::FindObject(string tag)
{
	return FindFirstObjectByTag(tag);
}

GameObject* ObjectManager::FindObjectByName(const string& name)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetName() == name)
		{
			return *itr;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetName() == name)
		{
			return *itr;
		}
	}

	return nullptr;
}

GameObject* ObjectManager::FindFirstObjectByTag(const string& tag)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			return *itr;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			return (*itr);
		}
	}

	return nullptr;
}

vector<GameObject*> ObjectManager::FindObjectsByTag(const string& tag)
{
	vector<GameObject*> matchedObjects;

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			matchedObjects.push_back(*itr);
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			matchedObjects.push_back(*itr);
		}
	}

	return matchedObjects;
}

bool ObjectManager::FindObject(GameObject* obj)
{
	return IsInObjectList(obj) && !IsPendingRemove(obj) && obj != nullptr && !obj->GetDestroy();
}

void ObjectManager::Initialize()
{
	m_objList = new list<GameObject*>();
	m_mouseInteractionBlocked = false;
	m_isClearInProgress = false;
	m_nextRuntimeObjectId = 1;
	NotifyObjectsCleared();
}

void ObjectManager::Release()
{
	FlushPendingObjects();

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr)
			(*itr)->Release();
	}
}

void ObjectManager::Update()
{
	FlushPendingObjects();

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr)
			continue;

		if ((*itr)->GetDestroy())
		{
			QueueDestroyObject(*itr);
			continue;
		}

		(*itr)->Update();

		if (((*itr)->GetDestroy()))
		{
			QueueDestroyObject(*itr);
		}
	}

	UpdateMouseInteraction();

	FlushPendingObjects();
}

void ObjectManager::UpdateMouseInteraction()
{
    if (IsMouseInteractionBlocked())
    {
        // 에디터 쪽에서 입력 차단 flag를 세운 동안에는 runtime hover 갱신을 멈춘다.
        GameObject* previousMouseHoverObject = m_currentMouseHoverObject;
        m_currentMouseHoverObject = nullptr;
        if (previousMouseHoverObject != nullptr &&
            !previousMouseHoverObject->GetDestroy() &&
            previousMouseHoverObject->GetActive())
        {
            previousMouseHoverObject->OnMouseHoverExit();
        }
        return;
    }

    GameObject* previousMouseHoverObject = m_currentMouseHoverObject;
    if (previousMouseHoverObject != nullptr && previousMouseHoverObject->GetDestroy())
    {
        // Hover 대상이 이미 파괴됐다면 HoverExit 없이 조용히 hover 상태만 해제한다.
        previousMouseHoverObject = nullptr;
    }

    m_currentMouseHoverObject = SanitizeMouseEventTarget(RaycastMouseToGameObject());

    if (previousMouseHoverObject != m_currentMouseHoverObject)
    {
        if (previousMouseHoverObject != nullptr &&
            !previousMouseHoverObject->GetDestroy() &&
            previousMouseHoverObject->GetActive())
        {
            previousMouseHoverObject->OnMouseHoverExit();
        }

        if (m_currentMouseHoverObject != nullptr &&
            !m_currentMouseHoverObject->GetDestroy() &&
            m_currentMouseHoverObject->GetActive())
        {
            m_currentMouseHoverObject->OnMouseHoverEnter();
        }
    }

    if (m_currentMouseHoverObject != nullptr &&
        !m_currentMouseHoverObject->GetDestroy() &&
        m_currentMouseHoverObject->GetActive())
    {
        m_currentMouseHoverObject->OnMouseHoverStay();
    }
}

void ObjectManager::FlushPendingObjects()
{
	if (m_isFlushing)
		return;

	m_isFlushing = true;

	vector<GameObject*> pendingAddObjects;
	pendingAddObjects.swap(m_pendingAddObjects);

	for (vector<GameObject*>::iterator itr = pendingAddObjects.begin(); itr != pendingAddObjects.end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr)
			continue;

		if (!IsInObjectList(obj))
			m_objList->push_back(obj);

		if (!obj->GetDestroy())
			obj->Start();
	}

	vector<GameObject*> pendingRemoveObjects;
	pendingRemoveObjects.swap(m_pendingRemoveObjects);

	for (vector<GameObject*>::iterator itr = pendingRemoveObjects.begin(); itr != pendingRemoveObjects.end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr)
			continue;

		for (list<GameObject*>::iterator listItr = m_objList->begin(); listItr != m_objList->end();)
		{
			if ((*listItr) == obj)
			{
				listItr = m_objList->erase(listItr);
				break;
			}
			else
			{
				listItr++;
			}
		}

		ReleaseAndDeleteObject(obj);
	}

	if (m_isClearInProgress && m_pendingRemoveObjects.empty())
	{
		m_isClearInProgress = false;
		NotifyObjectsCleared();
	}

	m_isFlushing = false;
}

void ObjectManager::Clear()
{
	if (m_Pthis)
	{
		m_Pthis->NotifyObjectsClearing();
		m_Pthis->m_isClearInProgress = true;

		for (vector<GameObject*>::iterator itr = m_Pthis->m_pendingAddObjects.begin(); itr != m_Pthis->m_pendingAddObjects.end(); itr++)
		{
			if (*itr != nullptr)
			{
				m_Pthis->QueueDestroyObjectTree(*itr);
			}
		}

		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			m_Pthis->QueueDestroyObjectTree(*itr);
		}
	}
}

int ObjectManager::Count()
{
	return m_objList->size();
}

list<GameObject*>* ObjectManager::GetObjList()
{
	return m_objList;
}

void ObjectManager::RegisterOnObjectDestroyRequested(const std::function<void(GameObject*)>& callback)
{
	m_objectDestroyRequestedCallbacks.push_back(callback);
}

void ObjectManager::RegisterOnObjectReleased(const std::function<void(GameObject*)>& callback)
{
	m_objectReleasedCallbacks.push_back(callback);
}

void ObjectManager::RegisterOnObjectsClearing(const std::function<void()>& callback)
{
	m_objectsClearingCallbacks.push_back(callback);
}

void ObjectManager::RegisterOnObjectsCleared(const std::function<void()>& callback)
{
	m_objectsClearedCallbacks.push_back(callback);
}

void ObjectManager::SetMouseInteractionBlocked(bool blocked)
{
	m_mouseInteractionBlocked = blocked;
}

bool ObjectManager::IsMouseInteractionBlocked() const
{
	return m_mouseInteractionBlocked;
}

std::string ObjectManager::SerializeObjects()
{
	return SerializeObjects(3);
}

std::string ObjectManager::SerializeObjects(int sceneVersion)
{
	std::ostringstream oss;
	oss << "[\n";

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr || obj->GetDestroy())
			continue;
		if (obj->GetTag() == "SceneChanger")
			continue;
		AssignRuntimeObjectId(obj);
	}

	bool isFirst = true;
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr || obj->GetDestroy())
			continue;
		if (obj->GetTag() == "SceneChanger")
			continue;

		if (!isFirst)
		{
			oss << ",\n";
		}
		int parentId = -1;
		if (obj->GetParent() != nullptr)
		{
			parentId = obj->GetParent()->GetId();
		}

		oss << "    " << obj->Serialize(obj->GetId(), parentId, sceneVersion);
		isFirst = false;
	}

	oss << "\n  ]";
	return oss.str();
}

static bool DeserializeComponents(GameObject* obj, const std::string& objectJson)
{
	std::string componentsArray;
	if (!SceneJson::ExtractArray(objectJson, "components", componentsArray))
	{
		return true;
	}

	vector<std::string> componentJsonList;
	if (!SceneJson::ExtractObjectsFromArray(componentsArray, componentJsonList))
	{
		std::cout << "SceneData load failed: invalid components array." << std::endl;
		return false;
	}

	struct PendingComponentData
	{
		Component* component = nullptr;
		std::string type;
		std::string dataJson;
	};

	vector<PendingComponentData> pendingComponents;
	for (vector<std::string>::iterator itr = componentJsonList.begin(); itr != componentJsonList.end(); itr++)
	{
		std::string type;
		std::string dataJson;
		if (!SceneJson::ReadString(*itr, "type", type) || !SceneJson::ExtractObject(*itr, "data", dataJson))
		{
			std::cout << "SceneData load failed: invalid component data." << std::endl;
			return false;
		}

		Component* component = ComponentFactory::GetInstance().Create(type, dataJson);
		if (component == nullptr)
		{
			std::cout << "SceneData unknown component type: " << type << std::endl;
			continue;
		}

		Component* addedComponent = obj->AddComponent(component, false, false);
		if (addedComponent == nullptr)
		{
			std::cout << "SceneData component skipped by policy: " << type << std::endl;
			continue;
		}

		PendingComponentData pendingComponent;
		pendingComponent.component = addedComponent;
		pendingComponent.type = type;
		pendingComponent.dataJson = dataJson;
		pendingComponents.push_back(pendingComponent);
	}

	for (vector<PendingComponentData>::iterator itr = pendingComponents.begin(); itr != pendingComponents.end(); itr++)
	{
		if (itr->component != nullptr)
		{
			itr->component->Initialize();
		}
	}

	for (vector<PendingComponentData>::iterator itr = pendingComponents.begin(); itr != pendingComponents.end(); itr++)
	{
		if (itr->component == nullptr)
		{
			continue;
		}

		if (!itr->component->Deserialize(itr->dataJson))
		{
			std::cout << "SceneData load failed: component deserialize failed: " << itr->type << std::endl;
			return false;
		}
	}
	return true;
}

bool ObjectManager::DeserializeObjects(const std::string& sceneJson)
{
	return DeserializeObjects(sceneJson, 3);
}

bool ObjectManager::DeserializeObjects(const std::string& sceneJson, int sceneVersion)
{
	RegisterEngineComponents();

	std::string objectsArray;
	if (!SceneJson::ExtractArray(sceneJson, "objects", objectsArray))
	{
		std::cout << "SceneData load failed: objects array not found." << std::endl;
		return false;
	}

	vector<std::string> objectJsonList;
	if (!SceneJson::ExtractObjectsFromArray(objectsArray, objectJsonList))
	{
		std::cout << "SceneData load failed: invalid objects array." << std::endl;
		return false;
	}

	vector<GameObjectSerializedData> objectDataList;
	for (vector<std::string>::iterator itr = objectJsonList.begin(); itr != objectJsonList.end(); itr++)
	{
		GameObjectSerializedData data;
		if (!GameObject::Deserialize(*itr, data, sceneVersion))
		{
			std::cout << "SceneData load failed: invalid GameObject data." << std::endl;
			return false;
		}
		objectDataList.push_back(data);
	}

	vector<GameObject*> createdObjects;
	std::unordered_map<int, GameObject*> objectMap;
	for (size_t i = 0; i < objectDataList.size(); i++)
	{
		GameObject* obj = GameObject::CreateFromSerializedData(objectDataList[i]);
		if (!DeserializeComponents(obj, objectJsonList[i]))
		{
			obj->Release();
			delete obj;
			for (vector<GameObject*>::iterator cleanupItr = createdObjects.begin(); cleanupItr != createdObjects.end(); cleanupItr++)
			{
				(*cleanupItr)->Release();
				delete (*cleanupItr);
			}
			return false;
		}

		if (objectDataList[i].objectId >= 0)
		{
			objectMap[objectDataList[i].objectId] = obj;
		}
		createdObjects.push_back(obj);
	}

	for (size_t i = 0; i < objectDataList.size(); i++)
	{
		if (objectDataList[i].parentId < 0)
			continue;

		std::unordered_map<int, GameObject*>::iterator parentItr = objectMap.find(objectDataList[i].parentId);
		if (parentItr == objectMap.end())
		{
			std::cout << "SceneData parent not found. Object is loaded as root: " << objectDataList[i].tag << std::endl;
			continue;
		}

		std::string parentRejectReason;
		if (!createdObjects[i]->CanSetParent(parentItr->second, &parentRejectReason))
		{
			std::cout << "SceneData parent skipped by policy. Object is loaded as root: "
				<< objectDataList[i].tag << " / reason: " << parentRejectReason << std::endl;
			continue;
		}

		createdObjects[i]->SetParent(parentItr->second);
	}

	for (vector<GameObject*>::iterator itr = createdObjects.begin(); itr != createdObjects.end(); itr++)
	{
		(*itr)->InitializeSet();
	}

	FlushPendingObjects();
	ResolveComponentReferences(objectMap);
	return true;
}

void ObjectManager::OnLBtnDown()
{
	GameObject* hitObject = SanitizeMouseEventTarget(m_currentMouseHoverObject);
	m_currentMouseHoverObject = hitObject;
	if (hitObject != nullptr)
	{
		hitObject->OnLBtnDown();
		if (hitObject->GetDestroy())
		{
			// 클릭 처리 중 파괴됐다면 이후 Up/Stay가 가지 않도록 hover 대상을 비운다.
			m_currentMouseHoverObject = nullptr;
		}
	}
}

void ObjectManager::OnLBtnUp()
{
	GameObject* hitObject = SanitizeMouseEventTarget(m_currentMouseHoverObject);
	m_currentMouseHoverObject = hitObject;
	if (hitObject != nullptr)
	{
		hitObject->OnLBtnUp();
		if (hitObject->GetDestroy())
		{
			// 클릭 처리 중 파괴됐다면 이후 Up/Stay가 가지 않도록 hover 대상을 비운다.
			m_currentMouseHoverObject = nullptr;
		}
	}
}

void ObjectManager::OnRBtnDown()
{
	GameObject* hitObject = SanitizeMouseEventTarget(m_currentMouseHoverObject);
	m_currentMouseHoverObject = hitObject;
	if (hitObject != nullptr)
	{
		hitObject->OnRBtnDown();
		if (hitObject->GetDestroy())
		{
			// 클릭 처리 중 파괴됐다면 이후 Up/Stay가 가지 않도록 hover 대상을 비운다.
			m_currentMouseHoverObject = nullptr;
		}
	}
}

void ObjectManager::OnRBtnUp()
{
	GameObject* hitObject = SanitizeMouseEventTarget(m_currentMouseHoverObject);
	m_currentMouseHoverObject = hitObject;
	if (hitObject != nullptr)
	{
		hitObject->OnRBtnUp();
		if (hitObject->GetDestroy())
		{
			// 클릭 처리 중 파괴됐다면 이후 Up/Stay가 가지 않도록 hover 대상을 비운다.
			m_currentMouseHoverObject = nullptr;
		}
	}
}

bool ObjectManager::GetLastMouseRaycastInfo(Ray& outRay, RaycastHit2D& outHit, bool& outHasHit) const
{
	if (!m_hasLastMouseRay)
	{
		outRay = Ray();
		outHit = RaycastHit2D();
		outHasHit = false;
		return false;
	}

	outRay = m_lastMouseRay;
	outHit = m_lastMouseRaycastHit;
	outHasHit = m_hasLastMouseRaycastHit;
	return true;
}
