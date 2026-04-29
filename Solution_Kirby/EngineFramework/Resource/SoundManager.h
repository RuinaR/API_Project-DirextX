#pragma once
#include "Resource/IResourceTypeManager.h"

struct SoundResource
{
	std::string key;
	std::string path;
};

class SoundManager : public IResourceTypeManager
{
public:
	SoundResource* GetSound(const std::string& assetKey);
	void ReleaseAllSounds();
	void ReleaseAllResources() override;

private:
	std::unordered_map<std::string, SoundResource*> m_soundMap;

	static std::string ResolvePath(const std::string& assetKey);
};
