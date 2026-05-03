#include "pch.h"
#include "FBXManager.h"
#include "Resource/AssetDatabase.h"

FbxResource* FBXManager::GetModel(const std::string& assetKey)
{
	if (assetKey.empty())
	{
		return nullptr;
	}

	std::unordered_map<std::string, FbxResource*>::iterator itr = m_modelMap.find(assetKey);
	if (itr != m_modelMap.end())
	{
		return itr->second;
	}

	FbxResource* resource = new FbxResource();
	resource->key = assetKey;
	resource->path = ResolvePath(assetKey);

	if (!resource->tool.Initialize())
	{
		delete resource;
		return nullptr;
	}

	if (!resource->tool.Load(resource->path.c_str(), &resource->models))
	{
		delete resource;
		return nullptr;
	}

	for (std::vector<Model>::iterator modelItr = resource->models.begin(); modelItr != resource->models.end(); modelItr++)
	{
		resource->tool.CreateVertexBuffer(&(*modelItr));
		resource->tool.CreateIndexBuffer(&(*modelItr));
	}

	resource->loaded = true;
	m_modelMap[assetKey] = resource;
	return resource;
}

void FBXManager::ReleaseAllModels()
{
	for (std::unordered_map<std::string, FbxResource*>::iterator itr = m_modelMap.begin(); itr != m_modelMap.end(); itr++)
	{
		for (std::vector<Model>::iterator modelItr = itr->second->models.begin(); modelItr != itr->second->models.end(); modelItr++)
		{
			if (modelItr->vertexBuffer)
			{
				modelItr->vertexBuffer->Release();
				modelItr->vertexBuffer = nullptr;
			}
			if (modelItr->indexBuffer)
			{
				modelItr->indexBuffer->Release();
				modelItr->indexBuffer = nullptr;
			}
		}
		delete itr->second;
	}
	m_modelMap.clear();
}

void FBXManager::ReleaseAllResources()
{
	ReleaseAllModels();
}

std::string FBXManager::ResolvePath(const std::string& assetKey)
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
