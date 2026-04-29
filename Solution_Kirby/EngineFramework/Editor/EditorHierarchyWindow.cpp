#include "pch.h"
#include "EditorHierarchyWindow.h"
#include "EditorObjectFactory.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "SceneDataManager.h"

namespace
{
	const char* kHierarchyDragDropPayload = "HierarchyGameObject";

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

	bool IsSameOrChild(GameObject* root, GameObject* target)
	{
		if (root == nullptr || target == nullptr)
		{
			return false;
		}

		if (root == target)
		{
			return true;
		}

		vector<GameObject*>* childList = root->GetChild();
		if (childList == nullptr)
		{
			return false;
		}

		for (vector<GameObject*>::iterator itr = childList->begin(); itr != childList->end(); itr++)
		{
			if (IsSameOrChild(*itr, target))
			{
				return true;
			}
		}
		return false;
	}

	void DeleteGameObjectFromHierarchy(GameObject* obj)
	{
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (objectManager == nullptr || obj == nullptr)
		{
			return;
		}

		if (IsSameOrChild(obj, objectManager->GetSelectedObject()))
		{
			objectManager->ClearSelectedObject();
		}
		objectManager->DestroyObject(obj);
		MarkCurrentSceneDirty();
	}

	bool ChangeParentInHierarchy(GameObject* child, GameObject* newParent)
	{
		if (child == nullptr)
		{
			return false;
		}

		if (newParent == child)
		{
			return false;
		}

		if (newParent != nullptr && IsSameOrChild(child, newParent))
		{
			return false;
		}

		if (child->GetParent() == newParent)
		{
			return false;
		}

		child->SetParent(newParent);
		MarkCurrentSceneDirty();
		return true;
	}

	void DrawHierarchyContextMenu(GameObject* obj)
	{
		if (obj == nullptr)
		{
			return;
		}

		if (ImGui::BeginPopupContextItem("GameObjectContext"))
		{
			if (ImGui::MenuItem("Make Root", nullptr, false, obj->GetParent() != nullptr))
			{
				ChangeParentInHierarchy(obj, nullptr);
			}
			if (ImGui::MenuItem("Delete"))
			{
				DeleteGameObjectFromHierarchy(obj);
			}
			ImGui::EndPopup();
		}
	}

	void DrawHierarchyDragDrop(GameObject* obj)
	{
		if (obj == nullptr)
		{
			return;
		}

		if (ImGui::BeginDragDropSource())
		{
			GameObject* payloadObject = obj;
			ImGui::SetDragDropPayload(kHierarchyDragDropPayload, &payloadObject, sizeof(payloadObject));
			ImGui::Text("%s", obj->GetTag().c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyDragDropPayload))
			{
				if (payload->DataSize == sizeof(GameObject*))
				{
					GameObject* draggedObject = *static_cast<GameObject* const*>(payload->Data);
					ChangeParentInHierarchy(draggedObject, obj);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}

void EditorHierarchyWindow::Draw()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return;
	}

	ImGui::Begin("Hierarchy");
	if (WindowFrame::GetInstance() != nullptr && WindowFrame::GetInstance()->GetRenderType() == RenderType::Edit)
	{
		const char* sceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
		const std::string sceneNameString = sceneName != nullptr ? sceneName : "";
		const bool isDirty = !sceneNameString.empty() && SceneDataManager::IsSceneDirty(sceneNameString);
		ImGui::Text("Scene: %s%s", sceneName != nullptr ? sceneName : "", isDirty ? " *" : "");
		ImGui::SameLine();
		ImGui::TextColored(isDirty ? ImVec4(1.0f, 0.75f, 0.15f, 1.0f) : ImVec4(0.35f, 1.0f, 0.35f, 1.0f), isDirty ? "Unsaved Changes" : "Saved");

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
		DrawGameObjectDeleteControls(objectManager->GetSelectedObject());

		if (ImGui::BeginPopup("CreateGameObjectPopup"))
		{
			GameObject* newObj = nullptr;
			if (ImGui::MenuItem("Create Empty"))
			{
				newObj = EditorObjectFactory::CreateEmptyGameObject();
			}
			if (ImGui::MenuItem("Create Sprite"))
			{
				newObj = EditorObjectFactory::CreateSpriteGameObject();
			}
			if (ImGui::MenuItem("Create UI Image"))
			{
				newObj = EditorObjectFactory::CreateUIImageGameObject();
			}
			if (ImGui::MenuItem("Create UI Button"))
			{
				newObj = EditorObjectFactory::CreateUIButtonGameObject();
			}
			if (ImGui::MenuItem("Create UI Text"))
			{
				newObj = EditorObjectFactory::CreateUITextGameObject();
			}

			if (newObj != nullptr)
			{
				objectManager->SetSelectedObject(newObj);
				MarkCurrentSceneDirty();
			}
			ImGui::EndPopup();
		}
		ImGui::Separator();
	}

	if (WindowFrame::GetInstance() != nullptr && WindowFrame::GetInstance()->GetRenderType() == RenderType::Edit)
	{
		ImGui::Selectable("Drop Here To Make Root", false);
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyDragDropPayload))
			{
				if (payload->DataSize == sizeof(GameObject*))
				{
					GameObject* draggedObject = *static_cast<GameObject* const*>(payload->Data);
					ChangeParentInHierarchy(draggedObject, nullptr);
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::Separator();
	}

	list<GameObject*>* objList = objectManager->GetObjList();
	if (objList != nullptr)
	{
		int visibleIndex = 0;
		for (list<GameObject*>::iterator itr = objList->begin(); itr != objList->end(); itr++)
		{
			if ((*itr) == nullptr || (*itr)->GetDestroy())
			{
				continue;
			}

			if ((*itr)->GetParent() != nullptr)
			{
				continue;
			}

			DrawGameObjectNode(*itr, 0, visibleIndex);
		}
	}
	ImGui::End();
}

void EditorHierarchyWindow::DrawGameObjectNode(GameObject* obj, int depth, int& visibleIndex)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr || obj == nullptr || obj->GetDestroy())
	{
		return;
	}

	char str[128];
	std::string prefix;
	if (depth > 0)
	{
		prefix = "  ";
		for (int i = 0; i < depth; i++)
		{
			prefix.append(" ");
		}
		prefix.append("L ");
	}

	visibleIndex++;
	sprintf_s(str, 128, "%s%d. %s", prefix.c_str(), visibleIndex, obj->GetTag().c_str());
	ImGui::PushID(obj);
	if (ImGui::Selectable(str, objectManager->GetSelectedObject() == obj))
	{
		objectManager->SetSelectedObject(obj);
	}
	DrawHierarchyDragDrop(obj);
	DrawHierarchyContextMenu(obj);
	ImGui::PopID();

	ProcessChildNode(obj, depth + 1);
}

void EditorHierarchyWindow::ProcessChildNode(GameObject* obj, int depth)
{
	if (obj == nullptr || obj->GetDestroy())
	{
		return;
	}

	vector<GameObject*>* childList = obj->GetChild();
	if (childList == nullptr)
	{
		return;
	}

	int visibleIndex = 0;
	for (vector<GameObject*>::iterator node = childList->begin(); node != childList->end(); node++)
	{
		if ((*node) == nullptr || (*node)->GetDestroy())
		{
			continue;
		}
		DrawGameObjectNode(*node, depth, visibleIndex);
	}
}

void EditorHierarchyWindow::DrawGameObjectDeleteControls(GameObject* obj)
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
		DeleteGameObjectFromHierarchy(obj);
	}
}
