#pragma once

#include "Resource/AssetDatabase.h"

class EditorAssetField
{
public:
	static bool Draw(const char* label, AssetType assetType, std::string& assetKey);

private:
	static bool CanSelectType(AssetType assetType);
	static bool PassTypeFilter(const AssetInfo& asset, AssetType assetType);
	static std::string BuildPreviewText(const std::string& assetKey);
};
