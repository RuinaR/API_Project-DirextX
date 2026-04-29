#pragma once

#include "Resource/IResourceTypeManager.h"
#include "FbxTool.h"

struct FbxResource
{
	std::string key;
	std::string path;
	FbxTool tool;
	std::vector<Model> models;
	bool loaded = false;

	FbxResource() = default;
	FbxResource(const FbxResource&) = delete;
	FbxResource& operator=(const FbxResource&) = delete;
};

class FBXManager : public IResourceTypeManager
{
public:
	FbxResource* GetModel(const std::string& assetKey);
	void ReleaseAllModels();
	void ReleaseAllResources() override;

private:
	std::unordered_map<std::string, FbxResource*> m_modelMap;

	static std::string ResolvePath(const std::string& assetKey);
};
