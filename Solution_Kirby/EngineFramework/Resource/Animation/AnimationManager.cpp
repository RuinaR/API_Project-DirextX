#include "pch.h"
#include "AnimationManager.h"
#include "ImageRender.h"
#include "Resource/Texture/TextureManager.h"

namespace
{
	std::wstring GetExecutableDirectoryW()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, path, MAX_PATH);
		std::wstring executePath = path;
		return executePath.substr(0, executePath.find_last_of(L"\\/"));
	}

	std::string GetExecutableDirectoryA()
	{
		char path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, path, MAX_PATH);
		std::string executePath = path;
		return executePath.substr(0, executePath.find_last_of("\\/"));
	}

	bool FileExistsA(const std::string& path)
	{
		DWORD attributes = GetFileAttributesA(path.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	bool DirectoryExistsW(const std::wstring& path)
	{
		DWORD attributes = GetFileAttributesW(path.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
}

Animation AnimationManager::GetAnimation(const std::wstring& folderName, float time, TextureManager* textureManager)
{
	return GetAnimation(folderName, time, textureManager, false);
}

Animation AnimationManager::GetAnimation(const std::wstring& folderName, float time, TextureManager* textureManager, bool useMagentaColorKey)
{
	const std::string animationKey = BuildAnimationKey(folderName, time)
		+ (useMagentaColorKey ? "#magenta" : "#opaque");
	std::unordered_map<std::string, Animation>::iterator cached = m_animationMap.find(animationKey);
	if (cached != m_animationMap.end())
	{
		return cached->second;
	}

	std::wstring currentDirectory = ResolveAssetDirectory(folderName);
	std::vector<IDirect3DTexture9*> textures;
	std::wstring searchPath = currentDirectory + L"\\*.*";
	WIN32_FIND_DATAW fileData;
	HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::wstring fileName = fileData.cFileName;
			if (IsImageFile(fileName))
			{
				std::wstring filePath = currentDirectory + L"\\" + fileName;
				IDirect3DTexture9* texture = textureManager != nullptr
					? textureManager->GetTexture(ConvertToString(filePath), useMagentaColorKey)
					: nullptr;

				if (texture)
				{
					textures.push_back(texture);
				}
				else
				{
					std::cout << "Texture load failed: " << ConvertToString(filePath) << std::endl;
				}
			}
		} while (FindNextFileW(hFind, &fileData) != 0);
		FindClose(hFind);
	}
	else
	{
		std::wcout << L"Animation folder not found: " << folderName << std::endl;
	}

	Animation newAnim;
	newAnim.textures = textures;
	newAnim.time = time;
	newAnim.sourcePath = ConvertToString(folderName);
	newAnim.identity = m_nextIdentity++;
	m_animationMap[animationKey] = newAnim;
	return newAnim;
}

IDirect3DTexture9* AnimationManager::GetTexture(const std::string& path, TextureManager* textureManager)
{
	if (textureManager == nullptr)
	{
		return nullptr;
	}

	const std::string searchPath = ResolveTexturePath(path);
	IDirect3DTexture9* texture = textureManager->GetTexture(searchPath);
	if (!texture)
	{
		std::cout << "Failed to load texture: " << searchPath << std::endl;
	}
	return texture;
}

void AnimationManager::GetTexture(const std::string& path, TextureManager* textureManager, std::function<void(IDirect3DTexture9*)> func)
{
	if (textureManager == nullptr)
	{
		if (func)
		{
			func(nullptr);
		}
		return;
	}

	const std::string searchPath = ResolveTexturePath(path);
	textureManager->GetTexture(searchPath, func);
}

void AnimationManager::GetTexture(const std::string& path, TextureManager* textureManager, ImageRender* ir)
{
	GetTexture(path, textureManager, std::bind(&ImageRender::LoadTextureCallback, ir, std::placeholders::_1));
}

void AnimationManager::ReleaseAllResources()
{
	m_animationMap.clear();
}

bool AnimationManager::IsImageFile(const std::wstring& filename)
{
	size_t dotIndex = filename.find_last_of('.');
	if (dotIndex != std::wstring::npos)
	{
		std::wstring extension = filename.substr(dotIndex + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		return extension == L"bmp" || extension == L"jpg" || extension == L"jpeg" || extension == L"png";
	}
	return false;
}

std::wstring AnimationManager::ResolveAssetDirectory(const std::wstring& path)
{
	std::wstring currentDirectory = GetExecutableDirectoryW();
	std::wstring searchPath = currentDirectory + L"\\" + path;
	if (DirectoryExistsW(searchPath))
	{
		return searchPath;
	}

	std::wstring debugAssetPath = currentDirectory + L"\\..\\..\\Debug\\" + path;
	if (DirectoryExistsW(debugAssetPath))
	{
		return debugAssetPath;
	}

	std::wstring solutionAssetPath = currentDirectory + L"\\..\\..\\" + path;
	if (DirectoryExistsW(solutionAssetPath))
	{
		return solutionAssetPath;
	}

	return searchPath;
}

std::string AnimationManager::ResolveTexturePath(const std::string& path)
{
	std::string currentDirectory = GetExecutableDirectoryA();
	std::string searchPath = currentDirectory + "\\" + path;
	if (FileExistsA(searchPath))
	{
		return searchPath;
	}

	std::string debugAssetPath = currentDirectory + "\\..\\..\\Debug\\" + path;
	if (FileExistsA(debugAssetPath))
	{
		return debugAssetPath;
	}

	std::string solutionAssetPath = currentDirectory + "\\..\\..\\" + path;
	if (FileExistsA(solutionAssetPath))
	{
		return solutionAssetPath;
	}

	return searchPath;
}

std::string AnimationManager::BuildAnimationKey(const std::wstring& folderName, float time)
{
	std::ostringstream oss;
	oss << ConvertToString(folderName) << "#" << time;
	return oss.str();
}
