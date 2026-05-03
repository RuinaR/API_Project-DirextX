#include "pch.h"
#include "SceneDataManager.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "SceneSerializationService.h"
#include "SceneJsonUtility.h"

namespace
{
	// Temporary facade storage so deserialize paths can expose the currently
	// loading scene version to legacy/runtime callers during scene load.
	int& GetCurrentLoadingSceneVersionStorage()
	{
		static int sceneVersion = 3;
		return sceneVersion;
	}

	// Scene file lists are exposed without the .json extension.
	std::string TrimSceneFileExtension(const std::string& fileName)
	{
		const size_t extensionPos = fileName.rfind(".json");
		if (extensionPos == std::string::npos)
		{
			return fileName;
		}
		return fileName.substr(0, extensionPos);
	}

	// Reject whitespace-only scene names in validation.
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

	return true;
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
	// Workflow-facing facade: records the loading version, forwards deserialize
	// to SceneSerializationService, and keeps path-based failure logging here.
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

int SceneDataManager::GetCurrentLoadingSceneVersion()
{
	return GetCurrentLoadingSceneVersionStorage();
}
