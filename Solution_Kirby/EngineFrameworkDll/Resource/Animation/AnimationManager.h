#pragma once

#include "AnimationRender.h"
#include "Resource/IResourceTypeManager.h"

class TextureManager;

class AnimationManager : public IResourceTypeManager
{
public:
	Animation GetAnimation(const std::wstring& folderName, float time, TextureManager* textureManager);
	Animation GetAnimation(const std::wstring& folderName, float time, TextureManager* textureManager, bool useMagentaColorKey);
	IDirect3DTexture9* GetTexture(const std::string& path, TextureManager* textureManager);
	void GetTexture(const std::string& path, TextureManager* textureManager, std::function<void(IDirect3DTexture9*)> func);
	void GetTexture(const std::string& path, TextureManager* textureManager, ImageRender* ir);
	void ReleaseAllResources() override;

private:
	std::unordered_map<std::string, Animation> m_animationMap;
	int m_nextIdentity = 1000;

	static bool IsImageFile(const std::wstring& filename);
	static std::wstring ResolveAssetDirectory(const std::wstring& path);
	static std::string ResolveTexturePath(const std::string& path);
	static std::string BuildAnimationKey(const std::wstring& folderName, float time);
};
