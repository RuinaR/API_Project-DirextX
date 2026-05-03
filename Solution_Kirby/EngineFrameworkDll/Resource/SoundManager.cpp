#include "pch.h"
#include "SoundManager.h"
#include "Resource/AssetDatabase.h"

SoundResource* SoundManager::GetSound(const std::string& assetKey)
{
	if (assetKey.empty())
	{
		return nullptr;
	}

	std::unordered_map<std::string, SoundResource*>::iterator itr = m_soundMap.find(assetKey);
	if (itr != m_soundMap.end())
	{
		return itr->second;
	}

	SoundResource* resource = new SoundResource();
	resource->key = assetKey;
	resource->path = ResolvePath(assetKey);
	m_soundMap[assetKey] = resource;
	return resource;
}

void SoundManager::ReleaseAllSounds()
{
	for (std::unordered_map<std::string, SoundResource*>::iterator itr = m_soundMap.begin(); itr != m_soundMap.end(); itr++)
	{
		delete itr->second;
	}
	m_soundMap.clear();
}

void SoundManager::ReleaseAllResources()
{
	ReleaseAllSounds();
}

std::string SoundManager::ResolvePath(const std::string& assetKey)
{
	AssetDatabase* assetDatabase = AssetDatabase::GetInstance();
	if (assetDatabase == nullptr)
	{
		return assetKey;
	}

	const AssetInfo* asset = assetDatabase->FindByKey(assetKey);
	if (asset == nullptr || asset->path.empty())
	{
		return assetKey;
	}

	return asset->path;
}
