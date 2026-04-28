#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "DebugWindow.h"
#include "SceneJsonUtility.h"
#include "ImageRender.h"
#include "AnimationRender.h"
#include "FBXRender.h"
#include "BoxCollider.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UILabel.h"
#include "SceneDataManager.h"
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

void ObjectManager::QueueDestroyObject(GameObject* obj)
{
	if (obj == nullptr || IsPendingRemove(obj))
		return;

	obj->SetDestroy(true);
	m_pendingRemoveObjects.push_back(obj);
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

void ObjectManager::ProcessChildNode(GameObject* obj, int depth)
{
	if (obj == nullptr || obj->GetDestroy())
		return;

	char str[64];
	string d = "  ";
	for (int i = 0; i < depth; i++)
	{
		d.append(" ");
	}
	d.append("L");
	for (auto node = obj->GetChild()->begin(); node != obj->GetChild()->end(); node++)
	{
		if ((*node) == nullptr || (*node)->GetDestroy())
			continue;

		sprintf_s(str, 64, "%s %s", d.c_str(), (*node)->GetTag().c_str());
		ProcessChildNode(*node, depth + 1);
		if (ImGui::Selectable(str, m_selected == (*node)))
		{
			m_selected = *node;
		}
	}
}

void ObjectManager::ImguiUpdate()
{
	DrawHierarchy();

	if (m_selected != nullptr && !m_selected->GetDestroy())
	{
		DrawGameObjectInspector(m_selected);
	}
}

void ObjectManager::DrawHierarchy()
{
	char str[64];

	ImGui::Begin("Hierarchy");
	if (WindowFrame::GetInstance() != nullptr && WindowFrame::GetInstance()->GetRenderType() == RenderType::Edit)
	{
		const char* sceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
		const std::string sceneNameString = sceneName != nullptr ? sceneName : "";
		const bool isDirty = !sceneNameString.empty() && SceneDataManager::IsSceneDirty(sceneNameString);
		ImGui::Text("Scene: %s", sceneName != nullptr ? sceneName : "");
		ImGui::SameLine();
		ImGui::TextColored(isDirty ? ImVec4(1.0f, 0.75f, 0.15f, 1.0f) : ImVec4(0.35f, 1.0f, 0.35f, 1.0f), isDirty ? "Dirty" : "Saved");

		if (ImGui::Button("Save Scene"))
		{
			if (sceneName != nullptr && strlen(sceneName) > 0)
			{
				const bool saved = SceneDataManager::SaveCurrentSceneData(sceneNameString);
				std::cout << (saved ? "Save Scene succeeded: " : "Save Scene failed: ") << sceneName << std::endl;
			}
		}
		ImGui::Separator();
	}

	int i = 0;
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;

		i++;
		if ((*itr)->GetParent() != nullptr)
			continue;
		sprintf_s(str, 64, "-%d. %s", i, (*itr)->GetTag().c_str());
		if (ImGui::Selectable(str, m_selected == (*itr)))
		{
			m_selected = (*itr);
		}
		ProcessChildNode((*itr), 1);
	}
	ImGui::End();
}

void ObjectManager::DrawGameObjectInspector(GameObject* obj)
{
	if (obj == nullptr)
		return;

	ImGui::Begin("Inspector");

	char tagBuffer[128] = {};
	strcpy_s(tagBuffer, obj->GetTag().c_str());
	if (ImGui::InputText("Tag", tagBuffer, IM_ARRAYSIZE(tagBuffer)))
	{
		obj->SetTag(tagBuffer);
	}

	bool active = obj->GetActive();
	if (ImGui::Checkbox("Active", &active))
	{
		obj->SetActive(active);
	}

	D3DXVECTOR3 position = obj->Position();
	if (ImGui::DragFloat3("Position", &position.x, 1.0f))
	{
		obj->SetPosition(position);
	}

	D3DXVECTOR3 size = obj->Size3D();
	if (ImGui::DragFloat3("Size", &size.x, 1.0f, 0.0f, 10000.0f))
	{
		// TODO: Collider/Box2D body size is not automatically rebuilt when Size3D changes.
		obj->Size3D() = size;
	}

	D3DXVECTOR3 angle = { obj->GetAngleX(), obj->GetAngleY(), obj->GetAngleZ() };
	if (ImGui::DragFloat3("Angle", &angle.x, 0.1f))
	{
		obj->SetAngleX(angle.x);
		obj->SetAngleY(angle.y);
		obj->SetAngleZ(angle.z);
	}

	ImGui::Separator();
	ImGui::Text("Components");

	vector<Component*>* components = obj->GetComponentVec();
	if (components != nullptr)
	{
		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			DrawComponentInspector(*itr);
		}
	}

	ImGui::End();
}

void ObjectManager::DrawComponentInspector(Component* component)
{
	if (component == nullptr)
		return;

	ImGui::PushID(component);
	if (ImGui::TreeNodeEx(component->GetInspectorName(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		component->DrawInspector();
		ImGui::TreePop();
	}
	ImGui::PopID();
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
		m_Pthis->m_pendingRemoveObjects.clear();

		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			delete(*itr);
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

	QueueDestroyObject(obj);
	return true;
}

bool ObjectManager::DestroyObject(string tag)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			QueueDestroyObject(*itr);
			return true;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			QueueDestroyObject(*itr);
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
		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			m_Pthis->QueueDestroyObject(*itr);
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

std::string ObjectManager::SerializeObjects()
{
	std::ostringstream oss;
	oss << "[\n";

	std::map<GameObject*, int> objectIds;
	int nextObjectId = 1;
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr || obj->GetDestroy())
			continue;
		if (obj->GetTag() == "SceneChanger")
			continue;

		objectIds[obj] = nextObjectId;
		nextObjectId++;
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
		std::map<GameObject*, int>::iterator parentItr = objectIds.find(obj->GetParent());
		if (parentItr != objectIds.end())
		{
			parentId = parentItr->second;
		}

		oss << "    " << obj->Serialize(objectIds[obj], parentId);
		isFirst = false;
	}

	oss << "\n  ]";
	return oss.str();
}

static Component* CreateSerializableComponent(const std::string& type, const std::string& dataJson)
{
	if (type == "ImageRender")
		return new ImageRender(nullptr);
	if (type == "AnimationRender")
		return new AnimationRender(Animation());
	if (type == "FBXRender")
	{
		std::string fbxPath;
		SceneJson::ReadString(dataJson, "fbxPath", fbxPath);
		return new FBXRender(fbxPath);
	}
	if (type == "BoxCollider")
		return new BoxCollider(b2_staticBody);
	if (type == "UIImage")
		return new UIImage();
	if (type == "UIButton")
		return new UIButton();
	if (type == "UILabel")
		return new UILabel();

	std::cout << "SceneData unknown component type: " << type << std::endl;
	return nullptr;
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

	for (vector<std::string>::iterator itr = componentJsonList.begin(); itr != componentJsonList.end(); itr++)
	{
		std::string type;
		std::string dataJson;
		if (!SceneJson::ReadString(*itr, "type", type) || !SceneJson::ExtractObject(*itr, "data", dataJson))
		{
			std::cout << "SceneData load failed: invalid component data." << std::endl;
			return false;
		}

		Component* component = CreateSerializableComponent(type, dataJson);
		if (component == nullptr)
		{
			continue;
		}

		obj->AddComponent(component);
		if (!component->Deserialize(dataJson))
		{
			std::cout << "SceneData load failed: component deserialize failed: " << type << std::endl;
			return false;
		}
	}
	return true;
}

bool ObjectManager::DeserializeObjects(const std::string& sceneJson)
{
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
		if (!GameObject::Deserialize(*itr, data))
		{
			std::cout << "SceneData load failed: invalid GameObject data." << std::endl;
			return false;
		}
		objectDataList.push_back(data);
	}

	vector<GameObject*> createdObjects;
	std::map<int, GameObject*> objectMap;
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

		std::map<int, GameObject*>::iterator parentItr = objectMap.find(objectDataList[i].parentId);
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
	return true;
}

void ObjectManager::OnLBtnDown()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnLBtnDown();
	}
}

void ObjectManager::OnLBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnLBtnUp();
	}
}

void ObjectManager::OnRBtnDown()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnRBtnDown();
	}
}

void ObjectManager::OnRBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnRBtnUp();
	}
}
