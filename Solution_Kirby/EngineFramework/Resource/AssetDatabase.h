#pragma once
#include <string>
#include <vector>

enum class AssetType
{
	Texture,
	Model,
	Animation,
	Sound,
	Unknown
};

struct AssetInfo
{
	std::string key;
	std::string path;
	std::string fileName;
	AssetType type = AssetType::Unknown;
	std::string extension;
};

class AssetDatabase
{
public:
	static void Create();
	static AssetDatabase* GetInstance();
	static void Destroy();

	void Scan();
	void Scan(const std::string& rootPath);
	const std::vector<AssetInfo>& GetAssets() const;
	const AssetInfo* FindByKey(const std::string& key) const;
	const std::string& GetRootPath() const;
	int GetAssetCount() const;

	static const char* AssetTypeToString(AssetType type);

private:
	static AssetDatabase* m_Pthis;

	std::vector<AssetInfo> m_assets;
	std::string m_rootPath;

	void ScanDirectoryRecursive(const std::string& directoryPath, const std::string& rootPath);

	static AssetType GetAssetType(const std::string& extension);
	static bool IsAnimationFolderName(const std::string& folderName);
	static std::string FindDefaultRootPath();
	static std::string NormalizePath(const std::string& path);
	static std::string ToLower(std::string value);
};
