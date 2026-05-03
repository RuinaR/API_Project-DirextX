#include "pch.h"
#include "SceneDataManager.h"
#include "Editor/EditorSceneWorkflow.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "SceneSerializationService.h"
#include "SceneJsonUtility.h"

namespace
{
	// 로드 중 하위 역직렬화 경로가 현재 SceneData 버전을 조회할 수 있도록
	// 일시적으로 유지하는 facade용 저장소다.
	int& GetCurrentLoadingSceneVersionStorage()
	{
		static int sceneVersion = 3;
		return sceneVersion;
	}

	// scene file 목록을 UI에 노출할 때 .json 확장자를 제거한다.
	std::string TrimSceneFileExtension(const std::string& fileName)
	{
		const size_t extensionPos = fileName.rfind(".json");
		if (extensionPos == std::string::npos)
		{
			return fileName;
		}
		return fileName.substr(0, extensionPos);
	}

	// scene 이름 validation에서 공백-only 입력을 걸러내기 위한 helper다.
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
}

std::string SceneDataManager::GetExeDirectory()
{
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, path, MAX_PATH);
	std::string exePath = path;
	size_t pos = exePath.find_last_of("\\/");
	if (pos == std::string::npos)
	{
		return std::string(".");
	}
	return exePath.substr(0, pos);
}

std::string SceneDataManager::GetFullPath(const std::string& path)
{
	char fullPath[MAX_PATH] = { 0 };
	if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr) == 0)
	{
		return path;
	}
	return fullPath;
}

bool SceneDataManager::DirectoryExists(const std::string& path)
{
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool SceneDataManager::EnsureDirectory(const std::string& path)
{
	if (DirectoryExists(path))
	{
		return true;
	}

	return CreateDirectoryA(path.c_str(), nullptr) == TRUE || GetLastError() == ERROR_ALREADY_EXISTS;
}

std::string SceneDataManager::JoinPath(const std::string& lhs, const std::string& rhs)
{
	if (lhs.empty())
	{
		return rhs;
	}
	if (lhs.back() == '\\' || lhs.back() == '/')
	{
		return lhs + rhs;
	}
	return lhs + "\\" + rhs;
}

std::string SceneDataManager::GetSceneDataDirectory()
{
	std::string exeDir = GetExeDirectory();
	std::string sceneDataFolder = "SceneData";
	std::string exeCandidate = JoinPath(exeDir, sceneDataFolder);
	if (DirectoryExists(exeCandidate))
	{
		return exeCandidate;
	}

	std::string relativeSceneData = "..\\..\\SceneData";
	std::string solutionPath = JoinPath(exeDir, relativeSceneData);
	std::string solutionCandidate = GetFullPath(solutionPath);
	if (DirectoryExists(solutionCandidate))
	{
		return solutionCandidate;
	}

	EnsureDirectory(exeCandidate);
	return exeCandidate;
}

std::string SceneDataManager::GetSceneDataPath(const std::string& sceneName)
{
	std::string sceneDataDirectory = GetSceneDataDirectory();
	std::string fileName = sceneName + ".json";
	return JoinPath(sceneDataDirectory, fileName);
}

bool SceneDataManager::Exists(const std::string& sceneName)
{
	const std::string path = GetSceneDataPath(sceneName);
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool SceneDataManager::IsValidSceneName(const std::string& sceneName)
{
	if (sceneName.empty() || ContainsOnlyWhitespace(sceneName))
	{
		return false;
	}

	static const char* kInvalidSceneNameChars = "\\/:*?\"<>|";
	return sceneName.find_first_of(kInvalidSceneNameChars) == std::string::npos;
}

bool SceneDataManager::SaveInitialSceneData(const std::string& sceneName)
{
	return SaveCurrentSceneData(sceneName);
}

bool SceneDataManager::SaveSceneData(const std::string& sceneName)
{
	return SaveCurrentSceneData(sceneName);
}

bool SceneDataManager::SaveCurrentSceneData(const std::string& sceneName)
{
	ObjectManager::GetInstance()->FlushPendingObjects();
	const std::string json = SceneSerializationService::BuildSceneDataJson(
		sceneName,
		MainFrame::GetInstance(),
		Camera::GetInstance(),
		ObjectManager::GetInstance());
	if (!SceneSerializationService::WriteSceneDataFile(GetSceneDataPath(sceneName), json))
	{
		return false;
	}

	EditorSceneWorkflow::UpdateCapturedSnapshot(sceneName, json);
	ClearSceneDirty(sceneName);
	return true;
}

// 아래 snapshot/dirty API는 editor workflow 구현으로 전달하는 facade wrapper다.
bool SceneDataManager::CaptureSceneSnapshot(const std::string& sceneName)
{
	return EditorSceneWorkflow::CaptureSceneSnapshot(sceneName);
}

std::string SceneDataManager::GetCapturedSnapshot(const std::string& sceneName)
{
	return EditorSceneWorkflow::GetCapturedSnapshot(sceneName);
}

void SceneDataManager::MarkSceneDirty(const std::string& sceneName)
{
	EditorSceneWorkflow::MarkSceneDirty(sceneName);
}

void SceneDataManager::ClearSceneDirty(const std::string& sceneName)
{
	EditorSceneWorkflow::ClearSceneDirty(sceneName);
}

bool SceneDataManager::IsSceneDirty(const std::string& sceneName)
{
	return EditorSceneWorkflow::IsSceneDirty(sceneName);
}

bool SceneDataManager::SaveCapturedSnapshot(const std::string& sceneName)
{
	return EditorSceneWorkflow::SaveCapturedSnapshot(sceneName);
}

bool SceneDataManager::LoadSceneData(const std::string& sceneName)
{
	std::string sceneJson;
	if (!SceneSerializationService::ReadSceneDataFile(GetSceneDataPath(sceneName), &sceneJson))
	{
		return false;
	}

	return DeserializeSceneDataForWorkflow(sceneName, sceneJson);
}

bool SceneDataManager::DeserializeSceneDataForWorkflow(const std::string& sceneName, const std::string& sceneJson)
{
	// editor workflow가 scene json 역직렬화를 요청할 때 사용하는 facade 진입점이다.
	// loading scene version 저장소와 실패 로그는 여기서 관리하고,
	// 실제 역직렬화 코어는 SceneSerializationService로 위임된다.
	int sceneVersion = 3;
	SceneJson::ReadInt(sceneJson, "version", sceneVersion);
	GetCurrentLoadingSceneVersionStorage() = sceneVersion;
	if (!SceneSerializationService::DeserializeSceneDataJson(
		sceneJson,
		MainFrame::GetInstance(),
		Camera::GetInstance(),
		ObjectManager::GetInstance(),
		&sceneVersion))
	{
		std::cout << "SceneData load failed: " << GetSceneDataPath(sceneName) << std::endl;
		GetCurrentLoadingSceneVersionStorage() = 3;
		return false;
	}

	GetCurrentLoadingSceneVersionStorage() = 3;
	return true;
}

// New Scene은 editor 전용 workflow 구현으로 위임된다.
bool SceneDataManager::CreateNewScene(const std::string& sceneName)
{
	return EditorSceneWorkflow::CreateNewScene(sceneName);
}

std::vector<std::string> SceneDataManager::GetSceneFileList()
{
	std::vector<std::string> sceneNames;
	const std::string searchPattern = JoinPath(GetSceneDataDirectory(), "*.json");

	WIN32_FIND_DATAA findData = {};
	HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		return sceneNames;
	}

	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			continue;
		}

		sceneNames.push_back(TrimSceneFileExtension(findData.cFileName));
	} while (FindNextFileA(findHandle, &findData) == TRUE);

	FindClose(findHandle);
	std::sort(sceneNames.begin(), sceneNames.end());
	return sceneNames;
}

// Open Scene은 rollback/dirty/snapshot 복원을 포함한 editor workflow 구현으로 위임된다.
bool SceneDataManager::OpenSceneData(const std::string& sceneName)
{
	return EditorSceneWorkflow::OpenSceneData(sceneName);
}

// Save Scene As는 editor 전용 scene name/current scene 갱신 정책을 포함한 workflow로 위임된다.
bool SceneDataManager::SaveSceneDataAs(const std::string& sceneName)
{
	return EditorSceneWorkflow::SaveSceneDataAs(sceneName);
}

int SceneDataManager::GetCurrentLoadingSceneVersion()
{
	return GetCurrentLoadingSceneVersionStorage();
}
