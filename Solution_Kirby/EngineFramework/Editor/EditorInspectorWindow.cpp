#include "pch.h"
#include "EditorInspectorWindow.h"
#include "EditorObjectFactory.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "SceneDataManager.h"

namespace
{
	const char* kHierarchyGameObjectPayload = "HierarchyGameObject";

	void MarkCurrentSceneDirty()
	{
		if (WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
		{
			return;
		}

		const char* sceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
		if (sceneName == nullptr || strlen(sceneName) == 0)
		{
			return;
		}

		SceneDataManager::MarkSceneDirty(sceneName);
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

	std::string GetComponentHeaderName(Component* component)
	{
		if (component == nullptr)
		{
			return "Component";
		}

		ComponentFactory& factory = ComponentFactory::GetInstance();
		const std::vector<ComponentFactory::ComponentInfo>& registeredComponents = factory.GetRegisteredComponents();
		for (std::vector<ComponentFactory::ComponentInfo>::const_iterator itr = registeredComponents.begin(); itr != registeredComponents.end(); itr++)
		{
			if (IsSameComponentType(component, itr->typeName))
			{
				return itr->displayName.empty() ? itr->typeName : itr->displayName;
			}
		}

		const char* inspectorName = component->GetInspectorName();
		if (inspectorName == nullptr || strlen(inspectorName) == 0)
		{
			return "Component";
		}
		return inspectorName;
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
						EditorObjectFactory::ApplyDefaultComponentValues(obj, addedComponent);
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

bool EditorInspectorWindow::DrawGameObjectReferenceField(const char* label, GameObject*& ref, int& refObjectId)
{
	bool changed = false;
	const char* safeLabel = label != nullptr ? label : "GameObject";

	if (ref != nullptr && ref->GetDestroy())
	{
		ref = nullptr;
	}

	std::string displayText = "(None)";
	if (ref != nullptr)
	{
		displayText = ref->GetTag();
		displayText += " [id=" + std::to_string(ref->GetId()) + "]";
	}
	else if (refObjectId >= 0)
	{
		displayText = "(Missing) [id=" + std::to_string(refObjectId) + "]";
	}

	ImGui::PushID(safeLabel);
	ImGui::Text("%s", safeLabel);
	ImGui::SameLine();
	ImGui::Button(displayText.c_str(), ImVec2(220.0f, 0.0f));

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyGameObjectPayload))
		{
			if (payload->DataSize == sizeof(GameObject*))
			{
				GameObject* droppedObject = *static_cast<GameObject* const*>(payload->Data);
				const int droppedObjectId = droppedObject != nullptr ? droppedObject->GetId() : -1;
				if (ref != droppedObject || refObjectId != droppedObjectId)
				{
					ref = droppedObject;
					refObjectId = droppedObjectId;
					changed = true;
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	const bool hasReference = ref != nullptr || refObjectId >= 0;
	if (!hasReference)
	{
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Clear"))
	{
		ref = nullptr;
		refObjectId = -1;
		changed = true;
	}
	if (!hasReference)
	{
		ImGui::EndDisabled();
	}

	ImGui::PopID();
	return changed;
}

void EditorInspectorWindow::Draw()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return;
	}

	GameObject* selected = objectManager->GetSelectedObject();
	if (selected != nullptr && !selected->GetDestroy())
	{
		DrawGameObjectInspector(selected);
	}
}

void EditorInspectorWindow::DrawGameObjectInspector(GameObject* obj)
{
	if (obj == nullptr)
	{
		return;
	}

	const std::string beforeInspectorState = obj->Serialize();

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

	if (beforeInspectorState != obj->Serialize())
	{
		MarkCurrentSceneDirty();
	}

	ImGui::End();
}

bool EditorInspectorWindow::DrawComponentInspector(Component* component)
{
	if (component == nullptr)
	{
		return false;
	}

	ImGui::PushID(component);
	const bool isRequired = IsRequiredComponent(component);
	const std::string headerName = GetComponentHeaderName(component);
	const bool isOpen = ImGui::TreeNodeEx(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
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
