#pragma once

#include "AssetDatabase.h"

class ResourceBrowser
{
public:
	static void Draw();

private:
	static bool PassFilter(const AssetInfo& asset, int filterIndex);
	static AssetType FilterIndexToAssetType(int filterIndex);
	static const char* FilterIndexToLabel(int filterIndex);
};
