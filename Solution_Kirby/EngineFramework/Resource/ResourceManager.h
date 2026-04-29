#pragma once

#include "Resource/Texture/TextureManager.h"
#include "Resource/FBXManager.h"
#include "Resource/SoundManager.h"
#include "Resource/Animation/AnimationManager.h"

class ImageRender;

class ResourceManager
{
public:
	static void Create();
	static ResourceManager* GetInstance();
	static void Destroy();

	IDirect3DTexture9* GetTexture(const std::string& assetKey);
	IDirect3DTexture9* GetTexture(const std::string& assetKey, bool useMagentaColorKey);
	void GetTexture(const std::string& assetKey, std::function<void(IDirect3DTexture9*)> callback);
	void GetTexture(const std::string& assetKey, ImageRender* imageRender);
	Animation GetAnimation(const std::wstring& folderName, float time);
	FbxResource* GetModel(const std::string& assetKey);
	SoundResource* GetSound(const std::string& assetKey);

	void ReleaseAllResources();

private:
	static ResourceManager* m_Pthis;

	TextureManager m_textureManager;
	AnimationManager m_animationManager;
	FBXManager m_fbxManager;
	SoundManager m_soundManager;
};
