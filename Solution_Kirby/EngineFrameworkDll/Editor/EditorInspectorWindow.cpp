#include "pch.h"
#include "EditorInspectorWindow.h"
#include "EditorObjectFactory.h"
#include "EditorSelectionService.h"
#include "EditorSceneWorkflow.h"
#include "EditorAssetField.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "FBXRender.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "Rendering/Component/AnimationRender.h"
#include "Rendering/Component/ImageRender.h"
#include "Rendering/UI/UIImage.h"
#include "UIElement.h"
#include "SceneDataManager.h"

namespace
{
	const char* kHierarchyGameObjectPayload = "HierarchyGameObject";
	const char* kDefaultGameplayTag = "Untagged";
	std::string g_nameValidationMessage;

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

		EditorSceneWorkflow::MarkSceneDirty(sceneName);
	}

	std::string NormalizeTagInput(const char* input)
	{
		if (input == nullptr)
		{
			return kDefaultGameplayTag;
		}

		std::string normalized = input;
		bool hasNonWhitespace = false;
		for (std::string::const_iterator itr = normalized.begin(); itr != normalized.end(); ++itr)
		{
			if (!isspace(static_cast<unsigned char>(*itr)))
			{
				hasNonWhitespace = true;
				break;
			}
		}

		if (!hasNonWhitespace)
		{
			return kDefaultGameplayTag;
		}

		return normalized;
	}

	bool ContainsOnlyWhitespace(const std::string& text)
	{
		for (std::string::const_iterator itr = text.begin(); itr != text.end(); ++itr)
		{
			if (!isspace(static_cast<unsigned char>(*itr)))
			{
				return false;
			}
		}
		return true;
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

	bool HasUIElementComponent(GameObject* obj)
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
			if (dynamic_cast<UIElement*>(*itr) != nullptr)
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

	std::string GetComponentReferenceDisplayText(Component* component)
	{
		if (component == nullptr)
		{
			return "(None)";
		}

		GameObject* owner = component->GetGameObject();
		std::string displayText = GetComponentHeaderName(component);
		displayText += " @ ";
		displayText += owner != nullptr ? owner->GetName() : "(Detached)";
		return displayText;
	}

	bool IsComponentStillValid(Component* component)
	{
		if (component == nullptr)
		{
			return false;
		}

		GameObject* owner = component->GetGameObject();
		if (owner == nullptr || owner->GetDestroy())
		{
			return false;
		}

		vector<Component*>* components = owner->GetComponentVec();
		if (components == nullptr)
		{
			return false;
		}

		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); ++itr)
		{
			if (*itr == component)
			{
				return true;
			}
		}

		return false;
	}

	Component* FindCompatibleComponentOnGameObject(GameObject* gameObject, const char* expectedType)
	{
		if (gameObject == nullptr || gameObject->GetDestroy())
		{
			return nullptr;
		}

		vector<Component*>* components = gameObject->GetComponentVec();
		if (components == nullptr)
		{
			return nullptr;
		}

		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); ++itr)
		{
			Component* component = *itr;
			if (component == nullptr)
			{
				continue;
			}

			if (expectedType == nullptr || strlen(expectedType) == 0 || IsSameComponentType(component, expectedType))
			{
				return component;
			}
		}

		return nullptr;
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
				bool canAdd = !alreadyAdded;
				std::string blockReason;
				if (!alreadyAdded)
				{
					Component* previewComponent = factory.Create(info->typeName, "{}");
					if (previewComponent != nullptr)
					{
						canAdd = obj->CanAddComponent(previewComponent, &blockReason);
						previewComponent->Release();
						delete previewComponent;
					}
					else
					{
						canAdd = false;
						blockReason = "컴포넌트를 미리 생성할 수 없습니다.";
					}
				}

				const char* shortcutLabel = nullptr;
				if (alreadyAdded)
				{
					shortcutLabel = "Added";
				}
				else if (!canAdd)
				{
					shortcutLabel = "Blocked";
				}

				if (ImGui::MenuItem(info->displayName.c_str(), shortcutLabel, false, canAdd))
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

				if (!canAdd && ImGui::IsItemHovered() && !blockReason.empty())
				{
					ImGui::SetTooltip("%s", blockReason.c_str());
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}

	void DrawEditorAssetInspectorFields(Component* component)
	{
		if (component == nullptr)
		{
			return;
		}

		if (UIImage* uiImage = dynamic_cast<UIImage*>(component))
		{
			std::string selectedTextureKey = uiImage->GetTexturePath();
			if (EditorAssetField::Draw("Image Asset", AssetType::Texture, selectedTextureKey))
			{
				uiImage->SetTexture(selectedTextureKey);
			}
			return;
		}

		if (ImageRender* imageRender = dynamic_cast<ImageRender*>(component))
		{
			std::string selectedTextureKey = imageRender->GetTexturePath();
			if (EditorAssetField::Draw("Texture Asset", AssetType::Texture, selectedTextureKey))
			{
				imageRender->SetTexturePath(selectedTextureKey);
			}
			return;
		}

		if (AnimationRender* animationRender = dynamic_cast<AnimationRender*>(component))
		{
			std::string animationFolderKey = animationRender->GetAnimationFolderKey();
			if (EditorAssetField::Draw("Animation Folder", AssetType::Animation, animationFolderKey))
			{
				animationRender->SetAnimationFolderKey(animationFolderKey);
			}
			return;
		}

		if (FBXRender* fbxRender = dynamic_cast<FBXRender*>(component))
		{
			std::string selectedModelKey = fbxRender->GetModelPath();
			if (EditorAssetField::Draw("Model Asset", AssetType::Model, selectedModelKey))
			{
				fbxRender->ReloadModel(selectedModelKey);
			}
		}
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
		displayText = ref->GetName();
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

bool EditorInspectorWindow::DrawComponentReferenceField(const char* label, Component*& ref, const char* expectedType)
{
	bool changed = false;
	const char* safeLabel = label != nullptr ? label : "Component";

	if (!IsComponentStillValid(ref))
	{
		ref = nullptr;
	}

	const std::string displayText = GetComponentReferenceDisplayText(ref);

	ImGui::PushID(safeLabel);
	ImGui::Text("%s", safeLabel);
	ImGui::SameLine();
	ImGui::Button(displayText.c_str(), ImVec2(260.0f, 0.0f));

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyGameObjectPayload))
		{
			if (payload->DataSize == sizeof(GameObject*))
			{
				GameObject* droppedObject = *static_cast<GameObject* const*>(payload->Data);
				Component* compatibleComponent = FindCompatibleComponentOnGameObject(droppedObject, expectedType);
				if (compatibleComponent != nullptr && ref != compatibleComponent)
				{
					ref = compatibleComponent;
					changed = true;
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	const bool hasReferenceBeforeClear = (ref != nullptr);
	if (!hasReferenceBeforeClear)
	{
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Clear"))
	{
		ref = nullptr;
		changed = true;
	}
	if (!hasReferenceBeforeClear)
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

	GameObject* selected = EditorSelectionService::GetSelectedObject();
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
	bool inspectorChanged = false;

	ImGui::Begin("Inspector");

	char nameBuffer[128] = {};
	strcpy_s(nameBuffer, obj->GetName().c_str());
	if (ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer)))
	{
		const std::string newName = nameBuffer;
		ObjectManager* objectManager = ObjectManager::GetInstance();
		GameObject* duplicatedObject = objectManager != nullptr ? objectManager->FindObjectByName(newName) : nullptr;
		if (newName.empty() || ContainsOnlyWhitespace(newName))
		{
			g_nameValidationMessage = "Name cannot be empty.";
		}
		else if (duplicatedObject != nullptr && duplicatedObject != obj)
		{
			g_nameValidationMessage = "Name already exists.";
		}
		else
		{
			obj->SetName(newName);
			g_nameValidationMessage.clear();
			inspectorChanged = true;
		}
	}

	if (!g_nameValidationMessage.empty())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "%s", g_nameValidationMessage.c_str());
	}

	char tagBuffer[128] = {};
	strcpy_s(tagBuffer, obj->GetTag().c_str());
	if (ImGui::InputText("Tag", tagBuffer, IM_ARRAYSIZE(tagBuffer)))
	{
		obj->SetTag(NormalizeTagInput(tagBuffer));
		inspectorChanged = true;
	}

	bool active = obj->GetActive();
	if (ImGui::Checkbox("Active", &active))
	{
		obj->SetActive(active);
	}

	D3DXVECTOR3 position = obj->Position();
	const bool hasUIElementComponent = HasUIElementComponent(obj);
	if (hasUIElementComponent)
	{
		ImGui::BeginDisabled();
		ImGui::DragFloat3("Position", &position.x, 1.0f);
		ImGui::EndDisabled();
		ImGui::TextDisabled("UI 오브젝트의 Position은 anchor와 local offset으로 계산되는 결과값입니다.");
	}
	else if (ImGui::DragFloat3("Position", &position.x, 1.0f))
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
			if (DrawComponentInspector(*itr, &inspectorChanged))
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

	if (inspectorChanged || beforeInspectorState != obj->Serialize())
	{
		MarkCurrentSceneDirty();
	}

	ImGui::End();
}

bool EditorInspectorWindow::DrawComponentInspector(Component* component, bool* outInspectorChanged)
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
			const std::string beforeComponentState = component->SerializeWithRegisteredReferenceFields();
			component->DrawInspector();

			ReferenceFieldRegistry referenceFieldRegistry;
			component->RegisterReferenceFields(referenceFieldRegistry);

			const std::vector<GameObjectRefField>& gameObjectRefs = referenceFieldRegistry.GetGameObjectRefs();
			for (std::vector<GameObjectRefField>::const_iterator itr = gameObjectRefs.begin(); itr != gameObjectRefs.end(); ++itr)
			{
				if (itr->field == nullptr)
				{
					continue;
				}

				GameObject* referencedObject = *(itr->field);
				int referencedObjectId = referencedObject != nullptr ? referencedObject->GetId() : -1;
				if (DrawGameObjectReferenceField(itr->label, referencedObject, referencedObjectId))
				{
					*(itr->field) = referencedObject;
					if (outInspectorChanged != nullptr)
					{
						*outInspectorChanged = true;
					}
				}
			}

			const std::vector<ComponentRefField>& componentRefs = referenceFieldRegistry.GetComponentRefs();
			for (std::vector<ComponentRefField>::const_iterator itr = componentRefs.begin(); itr != componentRefs.end(); ++itr)
			{
				if (itr->field == nullptr)
				{
					continue;
				}

				Component* referencedComponent = *(itr->field);
				if (DrawComponentReferenceField(itr->label, referencedComponent, itr->expectedType))
				{
					*(itr->field) = referencedComponent;
					if (outInspectorChanged != nullptr)
					{
						*outInspectorChanged = true;
					}
				}
			}

			DrawEditorAssetInspectorFields(component);
			if (outInspectorChanged != nullptr && beforeComponentState != component->SerializeWithRegisteredReferenceFields())
			{
				*outInspectorChanged = true;
			}
		}
		ImGui::TreePop();
	}
	ImGui::PopID();

	return removeRequested && !isRequired;
}
