#include "pch.h"
#include "EditorHierarchyWindow.h"
#include "EditorObjectFactory.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "SceneDataManager.h"

namespace
{
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
}

void EditorHierarchyWindow::Draw()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return;
	}

	char str[64];

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

	list<GameObject*>* objList = objectManager->GetObjList();
	if (objList != nullptr)
	{
		int i = 0;
		for (list<GameObject*>::iterator itr = objList->begin(); itr != objList->end(); itr++)
		{
			if ((*itr) == nullptr || (*itr)->GetDestroy())
			{
				continue;
			}

			i++;
			if ((*itr)->GetParent() != nullptr)
			{
				continue;
			}

			sprintf_s(str, 64, "-%d. %s", i, (*itr)->GetTag().c_str());
			ImGui::PushID(*itr);
			if (ImGui::Selectable(str, objectManager->GetSelectedObject() == (*itr)))
			{
				objectManager->SetSelectedObject(*itr);
			}
			if (ImGui::BeginPopupContextItem("GameObjectContext"))
			{
				if (ImGui::MenuItem("Delete"))
				{
					DeleteGameObjectFromHierarchy(*itr);
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
			ProcessChildNode((*itr), 1);
		}
	}
	ImGui::End();
}

void EditorHierarchyWindow::ProcessChildNode(GameObject* obj, int depth)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr || obj == nullptr || obj->GetDestroy())
	{
		return;
	}

	char str[64];
	std::string d = "  ";
	for (int i = 0; i < depth; i++)
	{
		d.append(" ");
	}
	d.append("L");

	vector<GameObject*>* childList = obj->GetChild();
	if (childList == nullptr)
	{
		return;
	}

	for (vector<GameObject*>::iterator node = childList->begin(); node != childList->end(); node++)
	{
		if ((*node) == nullptr || (*node)->GetDestroy())
		{
			continue;
		}

		sprintf_s(str, 64, "%s %s", d.c_str(), (*node)->GetTag().c_str());
		ProcessChildNode(*node, depth + 1);
		ImGui::PushID(*node);
		if (ImGui::Selectable(str, objectManager->GetSelectedObject() == (*node)))
		{
			objectManager->SetSelectedObject(*node);
		}
		if (ImGui::BeginPopupContextItem("GameObjectContext"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				DeleteGameObjectFromHierarchy(*node);
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
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
