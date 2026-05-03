#include "pch.h"
#include "EditorSceneWorkflow.h"
#include "Scene/SceneDataManager.h"
#include "Scene/SceneSerializationService.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "Rendering/Camera/Camera.h"
#include "WindowFrame.h"

namespace
{
	// editor가 씬 기준선을 비교할 수 있도록 scene별 snapshot을 유지한다.
	std::map<std::string, std::string>& GetCapturedSceneSnapshots()
	{
		static std::map<std::string, std::string> snapshots;
		return snapshots;
	}

	// editor dirty 상태를 scene name 기준으로 유지한다.
	std::map<std::string, bool>& GetSceneDirtyFlags()
	{
		static std::map<std::string, bool> dirtyFlags;
		return dirtyFlags;
	}
}

bool EditorSceneWorkflow::CaptureSceneSnapshot(const std::string& sceneName)
{
	ObjectManager::GetInstance()->FlushPendingObjects();
	UpdateCapturedSnapshot(
		sceneName,
		SceneSerializationService::BuildSceneDataJson(
			sceneName,
			MainFrame::GetInstance(),
			Camera::GetInstance(),
			ObjectManager::GetInstance()));
	ClearSceneDirty(sceneName);
	std::cout << "SceneData snapshot captured: " << sceneName << std::endl;
	return true;
}

std::string EditorSceneWorkflow::GetCapturedSnapshot(const std::string& sceneName)
{
	std::map<std::string, std::string>::iterator itr = GetCapturedSceneSnapshots().find(sceneName);
	if (itr == GetCapturedSceneSnapshots().end())
	{
		return std::string();
	}
	return itr->second;
}

void EditorSceneWorkflow::MarkSceneDirty(const std::string& sceneName)
{
	if (sceneName.empty())
	{
		return;
	}
	GetSceneDirtyFlags()[sceneName] = true;
}

void EditorSceneWorkflow::ClearSceneDirty(const std::string& sceneName)
{
	if (sceneName.empty())
	{
		return;
	}
	GetSceneDirtyFlags()[sceneName] = false;
}

bool EditorSceneWorkflow::IsSceneDirty(const std::string& sceneName)
{
	std::map<std::string, bool>::iterator dirtyItr = GetSceneDirtyFlags().find(sceneName);
	if (dirtyItr != GetSceneDirtyFlags().end() && dirtyItr->second)
	{
		return true;
	}

	const std::string snapshot = GetCapturedSnapshot(sceneName);
	if (snapshot.empty())
	{
		return false;
	}

	return snapshot != SceneSerializationService::BuildSceneDataJson(
		sceneName,
		MainFrame::GetInstance(),
		Camera::GetInstance(),
		ObjectManager::GetInstance());
}

bool EditorSceneWorkflow::SaveCapturedSnapshot(const std::string& sceneName)
{
	const std::string snapshot = GetCapturedSnapshot(sceneName);
	if (snapshot.empty())
	{
		std::cout << "SceneData snapshot not found: " << sceneName << std::endl;
		return false;
	}

	const bool saved = SceneSerializationService::WriteSceneDataFile(
		SceneDataManager::GetSceneDataPath(sceneName),
		snapshot);
	if (saved)
	{
		ClearSceneDirty(sceneName);
	}
	return saved;
}

bool EditorSceneWorkflow::CreateNewScene(const std::string& sceneName)
{
	if (sceneName.empty() || WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return false;
	}

	objectManager->Clear();
	objectManager->FlushPendingObjects();
	Camera::GetInstance()->InitializeView();
	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	MarkSceneDirty(sceneName);
	return true;
}

bool EditorSceneWorkflow::SaveSceneDataAs(const std::string& sceneName)
{
	if (WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	if (!SceneDataManager::IsValidSceneName(sceneName))
	{
		std::cout << "Save Scene As failed: invalid scene name: " << sceneName << std::endl;
		return false;
	}

	if (!SceneDataManager::SaveCurrentSceneData(sceneName))
	{
		return false;
	}

	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	ClearSceneDirty(sceneName);
	return true;
}

bool EditorSceneWorkflow::OpenSceneData(const std::string& sceneName)
{
	if (sceneName.empty() || WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return false;
	}

	std::string targetSceneJson;
	if (!SceneSerializationService::ReadSceneDataFile(SceneDataManager::GetSceneDataPath(sceneName), &targetSceneJson))
	{
		return false;
	}

	const char* currentSceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
	const std::string previousSceneName = currentSceneName != nullptr ? currentSceneName : "";
	const bool previousSceneDirty = !previousSceneName.empty() && IsSceneDirty(previousSceneName);
	const std::string previousSceneJson = previousSceneName.empty() ? std::string() : SceneSerializationService::BuildSceneDataJson(
		previousSceneName,
		MainFrame::GetInstance(),
		Camera::GetInstance(),
		ObjectManager::GetInstance());

	objectManager->Clear();
	objectManager->FlushPendingObjects();

	if (!SceneDataManager::DeserializeSceneDataForWorkflow(sceneName, targetSceneJson))
	{
		objectManager->Clear();
		objectManager->FlushPendingObjects();

		// target scene 역직렬화가 실패하면 이전 scene json으로 복원을 시도한다.
		if (!previousSceneJson.empty() && SceneDataManager::DeserializeSceneDataForWorkflow(previousSceneName, previousSceneJson))
		{
			WindowFrame::GetInstance()->SetCurrentSceneName(previousSceneName);
			// 복원 직후 snapshot을 다시 맞춘 뒤, 이전 dirty 상태를 복원한다.
			CaptureSceneSnapshot(previousSceneName);
			if (previousSceneDirty)
			{
				MarkSceneDirty(previousSceneName);
			}
			else
			{
				ClearSceneDirty(previousSceneName);
			}
		}
		return false;
	}

	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	CaptureSceneSnapshot(sceneName);
	ClearSceneDirty(sceneName);
	return true;
}

void EditorSceneWorkflow::UpdateCapturedSnapshot(const std::string& sceneName, const std::string& snapshot)
{
	GetCapturedSceneSnapshots()[sceneName] = snapshot;
}
