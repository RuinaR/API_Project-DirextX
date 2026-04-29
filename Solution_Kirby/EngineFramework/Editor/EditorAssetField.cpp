#include "pch.h"
#include "EditorAssetField.h"

bool EditorAssetField::Draw(const char* label, AssetType assetType, std::string& assetKey)
{
	if (!CanSelectType(assetType))
	{
		ImGui::Text("%s", label != nullptr ? label : "Asset");
		ImGui::SameLine();
		ImGui::TextDisabled("Unsupported asset type");
		return false;
	}

	bool changed = false;
	AssetDatabase* database = AssetDatabase::GetInstance();
	const std::string previewText = BuildPreviewText(assetKey);

	ImGui::PushID(label != nullptr ? label : "AssetField");
	if (ImGui::BeginCombo(label != nullptr ? label : "Asset", previewText.c_str()))
	{
		if (ImGui::Selectable("(None)", assetKey.empty()))
		{
			if (!assetKey.empty())
			{
				assetKey.clear();
				changed = true;
			}
		}

		if (database == nullptr)
		{
			ImGui::TextDisabled("AssetDatabase is not available.");
		}
		else
		{
			const std::vector<AssetInfo>& assets = database->GetAssets();
			int visibleCount = 0;
			for (std::vector<AssetInfo>::const_iterator itr = assets.begin(); itr != assets.end(); itr++)
			{
				if (!PassTypeFilter(*itr, assetType))
				{
					continue;
				}

				visibleCount++;
				const bool selected = assetKey == itr->key;
				std::string itemLabel = itr->fileName;
				if (!itr->extension.empty())
				{
					itemLabel += "##";
					itemLabel += itr->key;
				}

				if (ImGui::Selectable(itemLabel.c_str(), selected))
				{
					if (assetKey != itr->key)
					{
						assetKey = itr->key;
						changed = true;
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", itr->key.c_str());
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			if (visibleCount == 0)
			{
				ImGui::TextDisabled("No matching assets.");
			}
		}

		ImGui::EndCombo();
	}

	if (!assetKey.empty())
	{
		ImGui::TextDisabled("%s", assetKey.c_str());
	}
	ImGui::PopID();

	return changed;
}

bool EditorAssetField::CanSelectType(AssetType assetType)
{
	return assetType == AssetType::Texture
		|| assetType == AssetType::Model
		|| assetType == AssetType::Animation
		|| assetType == AssetType::Sound;
}

bool EditorAssetField::PassTypeFilter(const AssetInfo& asset, AssetType assetType)
{
	return asset.type == assetType;
}

std::string EditorAssetField::BuildPreviewText(const std::string& assetKey)
{
	if (assetKey.empty())
	{
		return "(None)";
	}

	AssetDatabase* database = AssetDatabase::GetInstance();
	if (database == nullptr)
	{
		return assetKey;
	}

	const AssetInfo* asset = database->FindByKey(assetKey);
	if (asset == nullptr)
	{
		return assetKey;
	}

	if (!asset->fileName.empty())
	{
		return asset->fileName;
	}
	return asset->key;
}
