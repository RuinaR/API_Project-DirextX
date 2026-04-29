#include "pch.h"
#include "ResourceManager.h"
#include "ImageRender.h"

ResourceManager* ResourceManager::m_Pthis = nullptr;

void ResourceManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new ResourceManager();
	}
}

ResourceManager* ResourceManager::GetInstance()
{
	return m_Pthis;
}

void ResourceManager::Destroy()
{
	if (m_Pthis)
	{
		m_Pthis->ReleaseAllResources();
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

IDirect3DTexture9* ResourceManager::GetTexture(const std::string& assetKey)
{
	return m_textureManager.GetTexture(assetKey);
}

IDirect3DTexture9* ResourceManager::GetTexture(const std::string& assetKey, bool useMagentaColorKey)
{
	return m_textureManager.GetTexture(assetKey, useMagentaColorKey);
}

void ResourceManager::GetTexture(const std::string& assetKey, std::function<void(IDirect3DTexture9*)> callback)
{
	m_textureManager.GetTexture(assetKey, callback);
}

void ResourceManager::GetTexture(const std::string& assetKey, ImageRender* imageRender)
{
	m_animationManager.GetTexture(assetKey, &m_textureManager, imageRender);
}

Animation ResourceManager::GetAnimation(const std::wstring& folderName, float time)
{
	return m_animationManager.GetAnimation(folderName, time, &m_textureManager);
}

FbxResource* ResourceManager::GetModel(const std::string& assetKey)
{
	return m_fbxManager.GetModel(assetKey);
}

SoundResource* ResourceManager::GetSound(const std::string& assetKey)
{
	return m_soundManager.GetSound(assetKey);
}

void ResourceManager::ReleaseAllResources()
{
	m_fbxManager.ReleaseAllResources();
	m_soundManager.ReleaseAllResources();
	m_animationManager.ReleaseAllResources();
	m_textureManager.ReleaseAllResources();
}
