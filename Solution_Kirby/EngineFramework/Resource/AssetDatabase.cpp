#include "pch.h"
#include "AssetDatabase.h"

namespace
{
	bool DirectoryExists(const std::string& path)
	{
		DWORD attributes = GetFileAttributesA(path.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	std::string JoinPath(const std::string& lhs, const std::string& rhs)
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

	std::string GetExeDirectory()
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

	std::string GetFullPath(const std::string& path)
	{
		char fullPath[MAX_PATH] = { 0 };
		if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr) == 0)
		{
			return path;
		}
		return fullPath;
	}

	std::string GetExtension(const std::string& fileName)
	{
		const size_t pos = fileName.find_last_of('.');
		if (pos == std::string::npos)
		{
			return std::string();
		}
		return fileName.substr(pos);
	}

	std::string GetFileName(const std::string& path)
	{
		const size_t pos = path.find_last_of("\\/");
		if (pos == std::string::npos)
		{
			return path;
		}
		return path.substr(pos + 1);
	}

	std::string MakeRelativePath(const std::string& fullPath, const std::string& rootPath)
	{
		if (fullPath.find(rootPath) != 0)
		{
			return GetFileName(fullPath);
		}

		std::string relative = fullPath.substr(rootPath.length());
		while (!relative.empty() && (relative.front() == '/' || relative.front() == '\\'))
		{
			relative.erase(relative.begin());
		}
		return relative;
	}
}

AssetDatabase* AssetDatabase::m_Pthis = nullptr;

void AssetDatabase::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new AssetDatabase();
	}
}

AssetDatabase* AssetDatabase::GetInstance()
{
	return m_Pthis;
}

void AssetDatabase::Destroy()
{
	if (m_Pthis)
	{
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void AssetDatabase::Scan()
{
	Scan(FindDefaultRootPath());
}

void AssetDatabase::Scan(const std::string& rootPath)
{
	m_assets.clear();
	m_rootPath = NormalizePath(GetFullPath(rootPath));

	if (m_rootPath.empty() || !DirectoryExists(m_rootPath))
	{
		std::cout << "AssetDatabase scan skipped. Root not found: " << m_rootPath << std::endl;
		return;
	}

	ScanDirectoryRecursive(m_rootPath, m_rootPath);

	std::sort(m_assets.begin(), m_assets.end(), [](const AssetInfo& lhs, const AssetInfo& rhs)
		{
			return lhs.key < rhs.key;
		});

	std::cout << "AssetDatabase scanned: " << m_rootPath << " (" << m_assets.size() << " assets)" << std::endl;
}

const std::vector<AssetInfo>& AssetDatabase::GetAssets() const
{
	return m_assets;
}

const AssetInfo* AssetDatabase::FindByKey(const std::string& key) const
{
	const std::string normalizedKey = NormalizePath(key);
	for (std::vector<AssetInfo>::const_iterator itr = m_assets.begin(); itr != m_assets.end(); itr++)
	{
		if (itr->key == normalizedKey)
		{
			return &(*itr);
		}
	}
	return nullptr;
}

const std::string& AssetDatabase::GetRootPath() const
{
	return m_rootPath;
}

int AssetDatabase::GetAssetCount() const
{
	return static_cast<int>(m_assets.size());
}

void AssetDatabase::ScanDirectoryRecursive(const std::string& directoryPath, const std::string& rootPath)
{
	const std::string directoryName = GetFileName(directoryPath);
	if (IsFbmDirectoryName(directoryName))
	{
		return;
	}
	if (IsAnimDirectoryName(directoryName))
	{
		AddAnimationFolderAsset(directoryPath, rootPath);
		return;
	}

	const std::string searchPath = JoinPath(directoryPath, "*");
	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		const std::string fileName = findData.cFileName;
		if (fileName == "." || fileName == "..")
		{
			continue;
		}

		const std::string fullPath = NormalizePath(GetFullPath(JoinPath(directoryPath, fileName)));
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (IsFbmDirectoryName(fileName))
			{
				continue;
			}
			if (IsAnimDirectoryName(fileName))
			{
				AddAnimationFolderAsset(fullPath, rootPath);
				continue;
			}
			ScanDirectoryRecursive(fullPath, rootPath);
			continue;
		}

		const std::string extension = ToLower(GetExtension(fileName));
		AssetType type = GetAssetType(extension);

		AssetInfo info;
		info.key = NormalizePath(MakeRelativePath(fullPath, rootPath));
		info.path = fullPath;
		info.fileName = fileName;
		info.type = type;
		info.extension = extension;
		m_assets.push_back(info);
	} while (FindNextFileA(findHandle, &findData));

	FindClose(findHandle);
}

void AssetDatabase::AddAnimationFolderAsset(const std::string& directoryPath, const std::string& rootPath)
{
	const std::string key = NormalizePath(MakeRelativePath(directoryPath, rootPath));
	if (key.empty() || HasAssetKey(key))
	{
		return;
	}

	AssetInfo info;
	info.key = key;
	info.path = NormalizePath(directoryPath);
	info.fileName = GetFileName(directoryPath);
	info.type = AssetType::Animation;
	info.extension = "folder";
	m_assets.push_back(info);
}

bool AssetDatabase::HasAssetKey(const std::string& key) const
{
	const std::string normalizedKey = NormalizePath(key);
	for (std::vector<AssetInfo>::const_iterator itr = m_assets.begin(); itr != m_assets.end(); itr++)
	{
		if (itr->key == normalizedKey)
		{
			return true;
		}
	}
	return false;
}

const char* AssetDatabase::AssetTypeToString(AssetType type)
{
	switch (type)
	{
	case AssetType::Texture:
		return "Texture";
	case AssetType::Model:
		return "Model";
	case AssetType::Animation:
		return "Animation";
	case AssetType::Sound:
		return "Sound";
	default:
		return "Unknown";
	}
}

AssetType AssetDatabase::GetAssetType(const std::string& extension)
{
	const std::string ext = ToLower(extension);
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".dds")
	{
		return AssetType::Texture;
	}
	if (ext == ".fbx")
	{
		return AssetType::Model;
	}
	if (ext == ".anim")
	{
		return AssetType::Animation;
	}
	if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
	{
		return AssetType::Sound;
	}
	return AssetType::Unknown;
}

bool AssetDatabase::IsImageFileExtension(const std::string& extension)
{
	const std::string ext = ToLower(extension);
	return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".dds";
}

bool AssetDatabase::IsFbmDirectoryName(const std::string& directoryName)
{
	return ToLower(GetExtension(directoryName)) == ".fbm";
}

bool AssetDatabase::IsAnimDirectoryName(const std::string& directoryName)
{
	return ToLower(GetExtension(directoryName)) == ".anim";
}

std::string AssetDatabase::FindDefaultRootPath()
{
	const std::string exeDir = GetExeDirectory();
	const std::vector<std::string> candidates =
	{
		JoinPath(exeDir, "Resources"),
		GetFullPath(JoinPath(exeDir, "..\\..\\Resources")),
		GetFullPath(JoinPath(exeDir, "..\\..\\Debug")),
		exeDir
	};

	for (std::vector<std::string>::const_iterator itr = candidates.begin(); itr != candidates.end(); itr++)
	{
		if (DirectoryExists(*itr))
		{
			return *itr;
		}
	}
	return std::string();
}

std::string AssetDatabase::NormalizePath(const std::string& path)
{
	std::string normalized = path;
	std::replace(normalized.begin(), normalized.end(), '\\', '/');
	return normalized;
}

std::string AssetDatabase::ToLower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
		{
			return static_cast<char>(std::tolower(ch));
		});
	return value;
}
