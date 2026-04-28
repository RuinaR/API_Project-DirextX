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
#include "ComponentFactory.h"
ObjectManager* ObjectManager::m_Pthis = nullptr;

namespace
{
	void ApplyDefaultComponentValues(GameObject* obj, Component* component);
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

bool ObjectManager::IsGameObjectNameUsed(const string& name)
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

string ObjectManager::CreateUniqueGameObjectName(const string& baseName)
{
	if (!IsGameObjectNameUsed(baseName))
	{
		return baseName;
	}

	int index = 1;
	while (true)
	{
		std::ostringstream oss;
		oss << baseName << " (" << index << ")";
		const string candidate = oss.str();
		if (!IsGameObjectNameUsed(candidate))
		{
			return candidate;
		}
		index++;
	}
}

GameObject* ObjectManager::CreateEmptyGameObject()
{
	GameObject* obj = new GameObject();
	obj->SetTag(CreateUniqueGameObjectName("New GameObject"));
	obj->InitializeSet();
	return obj;
}

GameObject* ObjectManager::CreateSpriteGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("Sprite"));
	ImageRender* imageRender = new ImageRender(nullptr);
	Component* addedComponent = obj->AddComponent(imageRender);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* ObjectManager::CreateUIImageGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Image"));
	UIImage* image = new UIImage();
	Component* addedComponent = obj->AddComponent(image);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* ObjectManager::CreateUIButtonGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Button"));
	UIButton* button = new UIButton();
	Component* addedComponent = obj->AddComponent(button);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* ObjectManager::CreateUITextGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Text"));
	UILabel* label = new UILabel();
	Component* addedComponent = obj->AddComponent(label);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
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
		ImGui::PushID(*node);
		if (ImGui::Selectable(str, m_selected == (*node)))
		{
			m_selected = *node;
		}
		if (ImGui::BeginPopupContextItem("GameObjectContext"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				GameObject* deleteTarget = *node;
				if (IsSameOrChild(deleteTarget, m_selected))
				{
					m_selected = nullptr;
				}
				DestroyObject(deleteTarget);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
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
		ImGui::SameLine();
		if (ImGui::Button("Create"))
		{
			ImGui::OpenPopup("CreateGameObjectPopup");
		}
		ImGui::SameLine();
		DrawGameObjectDeleteControls(m_selected);

		if (ImGui::BeginPopup("CreateGameObjectPopup"))
		{
			GameObject* newObj = nullptr;
			if (ImGui::MenuItem("Create Empty"))
			{
				newObj = CreateEmptyGameObject();
			}
			if (ImGui::MenuItem("Create Sprite"))
			{
				newObj = CreateSpriteGameObject();
			}
			if (ImGui::MenuItem("Create UI Image"))
			{
				newObj = CreateUIImageGameObject();
			}
			if (ImGui::MenuItem("Create UI Button"))
			{
				newObj = CreateUIButtonGameObject();
			}
			if (ImGui::MenuItem("Create UI Text"))
			{
				newObj = CreateUITextGameObject();
			}

			if (newObj != nullptr)
			{
				m_selected = newObj;
			}
			ImGui::EndPopup();
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
		ImGui::PushID(*itr);
		if (ImGui::Selectable(str, m_selected == (*itr)))
		{
			m_selected = (*itr);
		}
		if (ImGui::BeginPopupContextItem("GameObjectContext"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				GameObject* deleteTarget = *itr;
				if (IsSameOrChild(deleteTarget, m_selected))
				{
					m_selected = nullptr;
				}
				DestroyObject(deleteTarget);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
		ProcessChildNode((*itr), 1);
	}
	ImGui::End();
}

void ObjectManager::DrawGameObjectDeleteControls(GameObject* obj)
{
	if (obj == nullptr || obj->GetDestroy())
	{
		ImGui::BeginDisabled();
		ImGui::Button("Delete");
		ImGui::EndDisabled();
		return;
	}

	if (ImGui::Button("Delete"))
	{
		GameObject* deleteTarget = obj;
		if (IsSameOrChild(deleteTarget, m_selected))
		{
			m_selected = nullptr;
		}
		DestroyObject(deleteTarget);
	}
}

namespace
{
	void ApplyDefaultComponentValues(GameObject* obj, Component* component)
	{
		if (obj == nullptr || component == nullptr)
		{
			return;
		}

		if (UIButton* button = dynamic_cast<UIButton*>(component))
		{
			const D3DXVECTOR2 size(180.0f, 48.0f);
			button->SetSize(&size);
			button->SetUseTexture(false);
			button->SetStateColors(
				D3DCOLOR_ARGB(255, 80, 120, 220),
				D3DCOLOR_ARGB(255, 100, 145, 240),
				D3DCOLOR_ARGB(255, 55, 90, 180));
			button->SetActionKey("");
			return;
		}

		if (UIImage* image = dynamic_cast<UIImage*>(component))
		{
			const D3DXVECTOR2 size(128.0f, 128.0f);
			image->SetSize(&size);
			image->SetUseTexture(false);
			image->SetColor(D3DCOLOR_ARGB(255, 80, 180, 220));
			return;
		}

		if (UILabel* label = dynamic_cast<UILabel*>(component))
		{
			const D3DXVECTOR2 size(240.0f, 48.0f);
			label->SetSize(&size);
			label->SetText(L"Text");
			label->SetFontSize(24);
			label->SetColor(D3DCOLOR_ARGB(255, 255, 255, 255));
			return;
		}

		if (ImageRender* imageRender = dynamic_cast<ImageRender*>(component))
		{
			obj->Size3D() = D3DXVECTOR3(128.0f, 128.0f, 1.0f);
			imageRender->SetUseTexture(false);
			imageRender->SetColor(D3DCOLOR_ARGB(255, 255, 255, 255));
			imageRender->SetRenderEnabled(true);
			return;
		}
	}

	bool IsSameComponentType(Component* component, const std::string& typeName)
	{
		if (component == nullptr)
		{
			return false;
		}

		const char* serializableType = component->GetSerializableType();
		if (serializableType != nullptr && strlen(serializableType) > 0)
		{
			return typeName == serializableType;
		}

		const std::string inspectorName = component->GetInspectorName();
		return inspectorName == typeName
			|| inspectorName == "class " + typeName
			|| inspectorName == "struct " + typeName;
	}

	bool HasComponentType(GameObject* obj, const std::string& typeName)
	{
		if (obj == nullptr)
		{
			return false;
		}

		vector<Component*>* components = obj->GetComponentVec();
		if (components == nullptr)
		{
			return false;
		}

		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			if (IsSameComponentType(*itr, typeName))
			{
				return true;
			}
		}
		return false;
	}

	bool IsRequiredComponent(Component* component)
	{
		if (component == nullptr)
		{
			return true;
		}

		const char* serializableType = component->GetSerializableType();
		if (serializableType != nullptr && strcmp(serializableType, "Transform") == 0)
		{
			return true;
		}

		const std::string inspectorName = component->GetInspectorName();
		return inspectorName == "Transform"
			|| inspectorName == "class Transform"
			|| inspectorName == "struct Transform";
	}

	void DrawAddComponentMenu(GameObject* obj)
	{
		if (obj == nullptr)
		{
			return;
		}

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (!ImGui::BeginPopup("AddComponentPopup"))
		{
			return;
		}

		ComponentFactory& factory = ComponentFactory::GetInstance();
		const std::vector<ComponentFactory::ComponentInfo>& registeredComponents = factory.GetRegisteredComponents();
		std::map<std::string, std::vector<const ComponentFactory::ComponentInfo*>> categorizedComponents;

		for (std::vector<ComponentFactory::ComponentInfo>::const_iterator itr = registeredComponents.begin(); itr != registeredComponents.end(); itr++)
		{
			if (!itr->canAddFromEditor)
			{
				continue;
			}
			categorizedComponents[itr->category].push_back(&(*itr));
		}

		if (categorizedComponents.empty())
		{
			ImGui::TextDisabled("No components registered");
			ImGui::EndPopup();
			return;
		}

		for (std::map<std::string, std::vector<const ComponentFactory::ComponentInfo*>>::iterator categoryItr = categorizedComponents.begin(); categoryItr != categorizedComponents.end(); categoryItr++)
		{
			if (!ImGui::BeginMenu(categoryItr->first.c_str()))
			{
				continue;
			}

			for (std::vector<const ComponentFactory::ComponentInfo*>::iterator infoItr = categoryItr->second.begin(); infoItr != categoryItr->second.end(); infoItr++)
			{
				const ComponentFactory::ComponentInfo* info = *infoItr;
				if (info == nullptr)
				{
					continue;
				}

				const bool alreadyAdded = HasComponentType(obj, info->typeName);
				if (ImGui::MenuItem(info->displayName.c_str(), alreadyAdded ? "Added" : nullptr, false, !alreadyAdded))
				{
					Component* component = factory.Create(info->typeName, "{}");
					if (component != nullptr)
					{
						Component* addedComponent = obj->AddComponent(component);
						ApplyDefaultComponentValues(obj, addedComponent);
					}
					else
					{
						std::cout << "Add Component failed: " << info->typeName << std::endl;
					}
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
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
	Component* componentToRemove = nullptr;
	if (components != nullptr)
	{
		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			if (DrawComponentInspector(*itr))
			{
				componentToRemove = *itr;
				break;
			}
		}
	}

	if (componentToRemove != nullptr)
	{
		obj->DeleteComponent(componentToRemove);
	}

	DrawAddComponentMenu(obj);

	ImGui::End();
}

bool ObjectManager::DrawComponentInspector(Component* component)
{
	if (component == nullptr)
		return false;

	ImGui::PushID(component);
	const bool isRequired = IsRequiredComponent(component);
	const bool isOpen = ImGui::TreeNodeEx(component->GetInspectorName(), ImGuiTreeNodeFlags_DefaultOpen);
	bool removeRequested = false;

	ImGui::SameLine();
	if (isRequired)
	{
		ImGui::BeginDisabled();
	}
	removeRequested = ImGui::SmallButton("Remove");
	if (isRequired)
	{
		ImGui::EndDisabled();
	}

	if (isOpen)
	{
		if (!removeRequested)
		{
			component->DrawInspector();
		}
		ImGui::TreePop();
	}
	ImGui::PopID();

	return removeRequested && !isRequired;
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

		Component* component = ComponentFactory::GetInstance().Create(type, dataJson);
		if (component == nullptr)
		{
			std::cout << "SceneData unknown component type: " << type << std::endl;
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
