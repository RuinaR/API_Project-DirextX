#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "SceneJsonUtility.h"
#include "ComponentFactory.h"
#include "Editor/EditorHierarchyWindow.h"
#include "Editor/EditorInspectorWindow.h"
#include "Resource/ResourceBrowser.h"
#include "Input/Mouse.h"
#include "Physics2D/Physics2D.h"

ObjectManager* ObjectManager::m_Pthis = nullptr;

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

bool ObjectManager::IsSameOrChild(GameObject* root, GameObject* target)
{
	if (root == nullptr || target == nullptr)
		return false;

	if (root == target)
		return true;

	vector<GameObject*>* childList = root->GetChild();
	if (childList == nullptr)
		return false;

	for (vector<GameObject*>::iterator itr = childList->begin(); itr != childList->end(); itr++)
	{
		if (IsSameOrChild(*itr, target))
			return true;
	}
	return false;
}

void ObjectManager::ReleaseAndDeleteObject(GameObject* obj)
{
	if (obj == nullptr)
		return;

	if (m_selected == obj)
		m_selected = nullptr;

	obj->Release();
	delete obj;
}

bool ObjectManager::HasGameObjectName(const string& name)
{
	if (m_objList != nullptr)
	{
		for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
		{
			if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == name)
			{
				return true;
			}
		}
	}

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == name)
		{
			return true;
		}
	}

	return false;
}

void ObjectManager::ImguiUpdate()
{
	EditorHierarchyWindow::Draw();
	if (WindowFrame::GetInstance() != nullptr && WindowFrame::GetInstance()->GetRenderType() == RenderType::Edit)
	{
		ResourceBrowser::Draw();
	}
	EditorInspectorWindow::Draw();
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

	if (IsSameOrChild(obj, m_selected))
	{
		m_selected = nullptr;
	}
	QueueDestroyObjectTree(obj);
	return true;
}

bool ObjectManager::DestroyObject(string tag)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			DestroyObject(*itr);
			return true;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			DestroyObject(*itr);
			return true;
		}
	}
	return false;
}

GameObject* ObjectManager::FindObject(string tag)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			return (*itr);
		}
	}
	return nullptr;
}

bool ObjectManager::FindObject(GameObject* obj)
{
	return IsInObjectList(obj) && !IsPendingRemove(obj) && obj != nullptr && !obj->GetDestroy();
}

void ObjectManager::Initialize()
{
	m_objList = new list<GameObject*>();
	m_selected = nullptr;
	m_nextRuntimeObjectId = 1;
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

	FlushPendingObjects();
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

	m_isFlushing = false;
}

void ObjectManager::Clear()
{
	if (m_Pthis)
	{
		m_Pthis->ClearSelectedObject();

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

GameObject* ObjectManager::GetSelectedObject()
{
	return m_selected;
}

void ObjectManager::SetSelectedObject(GameObject* obj)
{
	m_selected = obj;
}

void ObjectManager::ClearSelectedObject()
{
	m_selected = nullptr;
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

		obj->AddComponent(component, false, false);

		PendingComponentData pendingComponent;
		pendingComponent.component = component;
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
	GameObject* hitObject = RaycastMouseToGameObject();
	if (hitObject != nullptr)
	{
		hitObject->OnLBtnDown();
	}
}

void ObjectManager::OnLBtnUp()
{
	GameObject* hitObject = RaycastMouseToGameObject();
	if (hitObject != nullptr)
	{
		hitObject->OnLBtnUp();
	}
}

void ObjectManager::OnRBtnDown()
{
	GameObject* hitObject = RaycastMouseToGameObject();
	if (hitObject != nullptr)
	{
		hitObject->OnRBtnDown();
	}
}

void ObjectManager::OnRBtnUp()
{
	GameObject* hitObject = RaycastMouseToGameObject();
	if (hitObject != nullptr)
	{
		hitObject->OnRBtnUp();
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
