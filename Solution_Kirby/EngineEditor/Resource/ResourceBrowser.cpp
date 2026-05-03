#include "pch.h"
#include "ResourceBrowser.h"

namespace
{
	int g_filterIndex = 0;
	std::string g_selectedAssetKey;

	const char* kFilterLabels[] =
	{
		"All",
		"Texture",
		"Model",
		"Animation",
		"Sound",
		"Unknown"
	};
}

void ResourceBrowser::Draw()
{
	AssetDatabase* database = AssetDatabase::GetInstance();

	ImGui::Begin("Resource Browser");

	if (database == nullptr)
	{
		ImGui::TextDisabled("AssetDatabase is not available.");
		ImGui::End();
		return;
	}

	ImGui::Text("Root: %s", database->GetRootPath().c_str());
	ImGui::SameLine();
	ImGui::Text("Assets: %d", database->GetAssetCount());

	if (ImGui::Button("Refresh"))
	{
		database->Scan();
		if (!g_selectedAssetKey.empty() && database->FindByKey(g_selectedAssetKey) == nullptr)
		{
			g_selectedAssetKey.clear();
		}
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(140.0f);
	if (ImGui::BeginCombo("Type", FilterIndexToLabel(g_filterIndex)))
	{
		for (int i = 0; i < IM_ARRAYSIZE(kFilterLabels); i++)
		{
			const bool selected = g_filterIndex == i;
			if (ImGui::Selectable(kFilterLabels[i], selected))
			{
				g_filterIndex = i;
			}
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();
	ImGui::BeginChild("AssetList", ImVec2(0.0f, 300.0f), true);
	ImGui::Columns(4, "AssetColumns");
	ImGui::Text("File");
	ImGui::NextColumn();
	ImGui::Text("Key");
	ImGui::NextColumn();
	ImGui::Text("Type");
	ImGui::NextColumn();
	ImGui::Text("Ext");
	ImGui::NextColumn();
	ImGui::Separator();

	const std::vector<AssetInfo>& assets = database->GetAssets();
	int visibleCount = 0;
	for (std::vector<AssetInfo>::const_iterator itr = assets.begin(); itr != assets.end(); itr++)
	{
		if (!PassFilter(*itr, g_filterIndex))
		{
			continue;
		}

		visibleCount++;
		ImGui::PushID(itr->key.c_str());
		const bool selected = g_selectedAssetKey == itr->key;
		if (ImGui::Selectable(itr->fileName.c_str(), selected))
		{
			g_selectedAssetKey = itr->key;
		}
		ImGui::NextColumn();
		ImGui::TextWrapped("%s", itr->key.c_str());
		ImGui::NextColumn();
		ImGui::Text("%s", AssetDatabase::AssetTypeToString(itr->type));
		ImGui::NextColumn();
		ImGui::Text("%s", itr->extension.c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	if (visibleCount == 0)
	{
		ImGui::TextDisabled("No assets match the current filter.");
	}

	ImGui::Columns(1);
	ImGui::EndChild();

	ImGui::Separator();
	ImGui::Text("Selected");
	const AssetInfo* selectedAsset = database->FindByKey(g_selectedAssetKey);
	if (selectedAsset == nullptr)
	{
		ImGui::TextDisabled("No asset selected.");
	}
	else
	{
		ImGui::Text("File: %s", selectedAsset->fileName.c_str());
		ImGui::Text("Type: %s", AssetDatabase::AssetTypeToString(selectedAsset->type));
		ImGui::Text("Extension: %s", selectedAsset->extension.c_str());
		ImGui::TextWrapped("Key: %s", selectedAsset->key.c_str());
		ImGui::TextWrapped("Path: %s", selectedAsset->path.c_str());
	}

	ImGui::End();
}

bool ResourceBrowser::PassFilter(const AssetInfo& asset, int filterIndex)
{
	if (filterIndex <= 0)
	{
		return true;
	}
	return asset.type == FilterIndexToAssetType(filterIndex);
}

AssetType ResourceBrowser::FilterIndexToAssetType(int filterIndex)
{
	switch (filterIndex)
	{
	case 1:
		return AssetType::Texture;
	case 2:
		return AssetType::Model;
	case 3:
		return AssetType::Animation;
	case 4:
		return AssetType::Sound;
	default:
		return AssetType::Unknown;
	}
}

const char* ResourceBrowser::FilterIndexToLabel(int filterIndex)
{
	if (filterIndex < 0 || filterIndex >= IM_ARRAYSIZE(kFilterLabels))
	{
		return kFilterLabels[0];
	}
	return kFilterLabels[filterIndex];
}
