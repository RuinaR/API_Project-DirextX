#include "pch.h"
#include "EditorHierarchyWindow.h"
#include "EditorBuildSettingsPanel.h"
#include "EditorObjectFactory.h"
#include "GameObject.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "Camera.h"
#include "Mouse.h"
#include "Physics2D/Physics2D.h"
#include "RenderManager.h"
#include "SceneDataManager.h"
#include <cfloat>

namespace
{
	const char* kHierarchyDragDropPayload = "HierarchyGameObject";
	char g_saveSceneAsName[128] = "NewScene";

	enum class PendingSceneAction
	{
		None,
		NewScene,
		OpenScene,
	};

	PendingSceneAction g_pendingSceneAction = PendingSceneAction::None;
	std::string g_pendingOpenSceneName;
	bool g_shouldExecutePendingSceneAction = false;
	bool g_shouldOpenSceneChangeConfirmPopup = false;
	bool g_enableDebugRaycast = false;
	float g_debugRaycastMaxDistance = 1000.0f;
	bool g_debugRaycastIncludeTriggers = false;
	bool g_debugRaycastShowNormal = true;
	const float kDebugRaycastHitCrossHalfSize = 5.0f;
	const float kDebugRaycastNormalLength = 20.0f;
	const D3DCOLOR kDebugRaycastColor = D3DCOLOR_XRGB(255, 0, 0);

	bool TryIntersectRayWithPhysicsPlane(const Ray& ray, D3DXVECTOR3& outPoint, float& outDistance)
	{
		const float epsilon = 0.000001f;

		if (fabs(ray.direction.z) <= epsilon)
		{
			if (fabs(ray.origin.z) > epsilon)
			{
				return false;
			}

			outPoint = ray.origin;
			outDistance = 0.0f;
			return true;
		}

		const float t = -ray.origin.z / ray.direction.z;
		if (t < 0.0f)
		{
			return false;
		}

		outPoint = ray.origin + (ray.direction * t);
		outDistance = t;
		return true;
	}

	std::string GetCurrentSceneName()
	{
		if (WindowFrame::GetInstance() == nullptr)
		{
			return std::string();
		}

		const char* sceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
		return sceneName != nullptr ? sceneName : "";
	}

	bool IsCurrentSceneDirty()
	{
		const std::string sceneName = GetCurrentSceneName();
		return !sceneName.empty() && SceneDataManager::IsSceneDirty(sceneName);
	}

	void ClearPendingSceneAction()
	{
		g_pendingSceneAction = PendingSceneAction::None;
		g_pendingOpenSceneName.clear();
		g_shouldExecutePendingSceneAction = false;
		g_shouldOpenSceneChangeConfirmPopup = false;
	}

	bool ExecutePendingSceneAction()
	{
		bool succeeded = false;
		if (g_pendingSceneAction == PendingSceneAction::NewScene)
		{
			succeeded = SceneDataManager::CreateNewScene();
			std::cout << (succeeded ? "New Scene created: " : "New Scene failed: ") << "NewScene" << std::endl;
		}
		else if (g_pendingSceneAction == PendingSceneAction::OpenScene)
		{
			succeeded = SceneDataManager::OpenSceneData(g_pendingOpenSceneName);
			std::cout << (succeeded ? "Open Scene succeeded: " : "Open Scene failed: ") << g_pendingOpenSceneName << std::endl;
		}

		ClearPendingSceneAction();
		return succeeded;
	}

	void QueuePendingSceneAction()
	{
		if (g_pendingSceneAction == PendingSceneAction::None)
		{
			return;
		}

		g_shouldExecutePendingSceneAction = true;
	}

	void RequestNewScene()
	{
		if (!IsCurrentSceneDirty())
		{
			g_pendingSceneAction = PendingSceneAction::NewScene;
			g_pendingOpenSceneName.clear();
			QueuePendingSceneAction();
			return;
		}

		g_pendingSceneAction = PendingSceneAction::NewScene;
		g_pendingOpenSceneName.clear();
		g_shouldOpenSceneChangeConfirmPopup = true;
	}

	void RequestOpenScene(const std::string& sceneName)
	{
		if (!IsCurrentSceneDirty())
		{
			g_pendingSceneAction = PendingSceneAction::OpenScene;
			g_pendingOpenSceneName = sceneName;
			QueuePendingSceneAction();
			return;
		}

		g_pendingSceneAction = PendingSceneAction::OpenScene;
		g_pendingOpenSceneName = sceneName;
		g_shouldOpenSceneChangeConfirmPopup = true;
	}

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

	void DrawCameraSection()
	{
		Camera* camera = Camera::GetInstance();
		if (camera == nullptr)
		{
			return;
		}

		if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		D3DXVECTOR3 position = camera->GetPos();
		if (ImGui::DragFloat3("Camera Position", &position.x, 1.0f))
		{
			camera->SetPos(position.x, position.y, position.z);
			MarkCurrentSceneDirty();
		}

		D3DXVECTOR3 rotation = camera->GetRotation();
		if (ImGui::DragFloat3("Camera Rotation", &rotation.x, 0.1f))
		{
			camera->SetRotation(&rotation);
			MarkCurrentSceneDirty();
		}

		int projectionMode = static_cast<int>(camera->GetProjectionMode());
		const char* projectionModeLabels[] = { "Orthographic", "Perspective" };
		if (ImGui::Combo("Projection Mode", &projectionMode, projectionModeLabels, IM_ARRAYSIZE(projectionModeLabels)))
		{
			camera->SetProjectionMode(static_cast<CameraProjectionMode>(projectionMode));
			MarkCurrentSceneDirty();
		}

		float fov = camera->GetFov();
		if (ImGui::DragFloat("FOV", &fov, 0.01f, 0.01f, D3DX_PI - 0.01f))
		{
			camera->SetFov(fov);
			MarkCurrentSceneDirty();
		}

		float orthographicSize = camera->GetOrthographicSize();
		if (ImGui::DragFloat("Orthographic Size", &orthographicSize, 1.0f, 1.0f, 10000.0f))
		{
			camera->SetOrthographicSize(orthographicSize);
			MarkCurrentSceneDirty();
		}

		float nearClip = camera->GetNearClip();
		if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f, 0.001f, 10000.0f))
		{
			camera->SetNearClip(nearClip);
			MarkCurrentSceneDirty();
		}

		float farClip = camera->GetFarClip();
		if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, 1.0f, 100000.0f))
		{
			camera->SetFarClip(farClip);
			MarkCurrentSceneDirty();
		}

		if (ImGui::Button("Reset Camera"))
		{
			camera->InitializeView();
			MarkCurrentSceneDirty();
		}

		RenderManager* renderManager = RenderManager::GetInstance();
		if (renderManager != nullptr)
		{
			bool colliderDebug = renderManager->IsColliderDebugVisible();
			if (ImGui::Checkbox("Show Collider Debug", &colliderDebug))
			{
				renderManager->SetColliderDebugVisible(colliderDebug);
			}
		}
	}

	void DrawRaycastDebugSection()
	{
		if (!ImGui::CollapsingHeader("Raycast Debug", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		ImGui::Checkbox("Enable Debug Raycast", &g_enableDebugRaycast);
		if (!g_enableDebugRaycast)
		{
			if (RenderManager::GetInstance() != nullptr)
			{
				RenderManager::GetInstance()->ClearImmediateDebugLines();
			}
			ImGui::TextDisabled("Enable to inspect the current mouse raycast.");
			return;
		}

		if (g_debugRaycastMaxDistance <= 0.0f)
		{
			g_debugRaycastMaxDistance = 1.0f;
		}
		if (ImGui::DragFloat("Max Distance", &g_debugRaycastMaxDistance, 10.0f, 1.0f, 100000.0f, "%.1f"))
		{
			if (g_debugRaycastMaxDistance <= 0.0f)
			{
				g_debugRaycastMaxDistance = 1.0f;
			}
		}
		ImGui::Checkbox("Include Triggers", &g_debugRaycastIncludeTriggers);
		ImGui::Checkbox("Show Hit Normal", &g_debugRaycastShowNormal);

		Mouse* mouse = Mouse::GetInstance();
		RenderManager* renderManager = RenderManager::GetInstance();
		if (mouse == nullptr || renderManager == nullptr)
		{
			ImGui::TextDisabled("Mouse or RenderManager unavailable");
			return;
		}

		const Ray ray = mouse->ScreenPointToRay();
		RaycastHit2D hit;
		const bool hasHit = Physics2D::Raycast(ray, hit, g_debugRaycastMaxDistance, g_debugRaycastIncludeTriggers);
		const D3DXVECTOR3 rayStart = ray.origin;
		D3DXVECTOR3 rayEnd = ray.origin + (ray.direction * g_debugRaycastMaxDistance);
		D3DXVECTOR3 planePoint;
		float planeDistance = 0.0f;
		if (TryIntersectRayWithPhysicsPlane(ray, planePoint, planeDistance))
		{
			if (planeDistance <= g_debugRaycastMaxDistance)
			{
				rayEnd = planePoint;
			}
		}
		if (hasHit)
		{
			rayEnd = hit.point;
		}

		renderManager->ClearImmediateDebugLines();
		renderManager->AddImmediateDebugLine(rayStart, rayEnd, kDebugRaycastColor);
		if (hasHit)
		{
			renderManager->AddImmediateDebugCross(hit.point, kDebugRaycastHitCrossHalfSize, kDebugRaycastColor);
			if (g_debugRaycastShowNormal)
			{
				renderManager->AddImmediateDebugLine(
					hit.point,
					hit.point + (hit.normal * kDebugRaycastNormalLength),
					kDebugRaycastColor);
			}
		}

		ImGui::Text("Ray Origin: %.2f, %.2f, %.2f", ray.origin.x, ray.origin.y, ray.origin.z);
		ImGui::Text("Ray Direction: %.3f, %.3f, %.3f", ray.direction.x, ray.direction.y, ray.direction.z);
		ImGui::Text("Hit: %s", hasHit ? "true" : "false");

		if (hasHit && hit.gameObject != nullptr)
		{
			ImGui::Text("Hit GameObject: %s", hit.gameObject->GetTag().c_str());
			ImGui::Text("Hit Distance: %.3f", hit.distance);
			ImGui::Text("Hit Point: %.2f, %.2f, %.2f", hit.point.x, hit.point.y, hit.point.z);
			ImGui::Text("Hit Normal: %.2f, %.2f, %.2f", hit.normal.x, hit.normal.y, hit.normal.z);
		}
		else
		{
			ImGui::Text("Hit GameObject: None");
			ImGui::Text("Hit Distance: N/A");
			ImGui::Text("Hit Point: N/A");
			ImGui::Text("Hit Normal: N/A (AABB approximation)");
		}
	}

	void DrawEditorPlaybackControls()
	{
		MainFrame* mainFrame = MainFrame::GetInstance();
		if (mainFrame == nullptr)
		{
			return;
		}

		const bool isPlaying = mainFrame->IsEditorPlaying();
		const bool isPaused = mainFrame->IsEditorPaused();

		if (isPlaying)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Play"))
		{
			mainFrame->SetEditorPlaying(true);
		}
		if (isPlaying)
		{
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		if (isPaused)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Pause"))
		{
			mainFrame->SetEditorPlaying(false);
		}
		if (isPaused)
		{
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		if (isPlaying)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Step"))
		{
			mainFrame->RequestEditorStep();
		}
		if (isPlaying)
		{
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		const char* playbackLabel = "Playback: Paused";
		ImVec4 playbackColor = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
		if (isPlaying)
		{
			playbackLabel = "Playback: Playing";
			playbackColor = ImVec4(0.35f, 1.0f, 0.35f, 1.0f);
		}
		else if (mainFrame->IsEditorStepRequested())
		{
			playbackLabel = "Playback: Step Pending";
			playbackColor = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
		}
		ImGui::TextColored(playbackColor, "%s", playbackLabel);
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
		DrawEditorPlaybackControls();

		if (ImGui::Button("Create"))
		{
			ImGui::OpenPopup("CreateGameObjectPopup");
		}
		ImGui::SameLine();
		DrawGameObjectDeleteControls(objectManager->GetSelectedObject());
		ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));

		if (ImGui::Button("New Scene"))
		{
			RequestNewScene();
		}
		ImGui::SameLine();
		if (ImGui::Button("Open Scene"))
		{
			ImGui::OpenPopup("OpenScenePopup");
		}
		ImGui::SameLine();
		if (ImGui::Button("Save Scene"))
		{
			if (sceneName != nullptr && strlen(sceneName) > 0)
			{
				const bool saved = SceneDataManager::SaveCurrentSceneData(sceneNameString);
				std::cout << (saved ? "Save Scene succeeded: " : "Save Scene failed: ") << sceneName << std::endl;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save Scene As"))
		{
			const char* currentSceneName = sceneName != nullptr && strlen(sceneName) > 0 ? sceneName : "NewScene";
			strcpy_s(g_saveSceneAsName, currentSceneName);
			ImGui::OpenPopup("SaveSceneAsPopup");
		}

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
		DrawOpenScenePopup();
		DrawSaveSceneAsPopup();
		ImGui::Separator();
		DrawCameraSection();
		ImGui::Separator();
		DrawRaycastDebugSection();
		ImGui::Separator();
		EditorBuildSettingsPanel::Draw();
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

	if (g_shouldOpenSceneChangeConfirmPopup)
	{
		ImGui::OpenPopup("ConfirmSceneChangePopup");
		g_shouldOpenSceneChangeConfirmPopup = false;
	}

	DrawSceneDirtyConfirmPopup();

	if (g_shouldExecutePendingSceneAction)
	{
		ExecutePendingSceneAction();
	}
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

void EditorHierarchyWindow::DrawOpenScenePopup()
{
	if (!ImGui::BeginPopup("OpenScenePopup"))
	{
		return;
	}

	const std::vector<std::string> sceneNames = SceneDataManager::GetSceneFileList();
	if (sceneNames.empty())
	{
		ImGui::TextDisabled("SceneData JSON not found.");
	}
	else
	{
		for (std::vector<std::string>::const_iterator itr = sceneNames.begin(); itr != sceneNames.end(); ++itr)
		{
			if (!ImGui::Selectable(itr->c_str()))
			{
				continue;
			}

			RequestOpenScene(*itr);
			ImGui::CloseCurrentPopup();
			break;
		}
	}

	ImGui::EndPopup();
}

void EditorHierarchyWindow::DrawSaveSceneAsPopup()
{
	if (!ImGui::BeginPopupModal("SaveSceneAsPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::Text("Scene Name");
	ImGui::InputText("##SaveSceneAsName", g_saveSceneAsName, IM_ARRAYSIZE(g_saveSceneAsName));
	ImGui::TextDisabled("Invalid chars: \\ / : * ? \" < > |");

	const std::string newSceneName = g_saveSceneAsName;
	const bool canSave = SceneDataManager::IsValidSceneName(newSceneName);
	if (!canSave)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Save"))
	{
		const bool saved = SceneDataManager::SaveSceneDataAs(newSceneName);
		std::cout << (saved ? "Save Scene As succeeded: " : "Save Scene As failed: ") << newSceneName << std::endl;
		if (saved)
		{
			ImGui::CloseCurrentPopup();
		}
	}

	if (!canSave)
	{
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void EditorHierarchyWindow::DrawSceneDirtyConfirmPopup()
{
	if (!ImGui::BeginPopupModal("ConfirmSceneChangePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::Text("현재 씬에 저장되지 않은 변경사항이 있습니다.");
	ImGui::Text("저장 후 계속할까요?");
	ImGui::Separator();

	if (ImGui::Button("Save"))
	{
		const std::string sceneName = GetCurrentSceneName();
		const bool saved = !sceneName.empty() && SceneDataManager::SaveCurrentSceneData(sceneName);
		std::cout << (saved ? "Save Scene succeeded: " : "Save Scene failed: ") << sceneName << std::endl;
		if (saved)
		{
			QueuePendingSceneAction();
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Discard"))
	{
		QueuePendingSceneAction();
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		ClearPendingSceneAction();
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
