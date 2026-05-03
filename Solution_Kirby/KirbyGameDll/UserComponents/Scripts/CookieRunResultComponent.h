#pragma once

#include "Component.h"

class UILabel;

class CookieRunResultComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunResultComponent";

	CookieRunResultComponent() = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void RegisterReferenceFields(ReferenceFieldRegistry& registry) override;

private:
	void ResolveFallbackReferences();
	void UpdateResultScoreUI();
	UILabel* GetResultScoreLabel() const;

	Component* m_resultScoreLabelComponent = nullptr;
	bool m_loggedResultEntry = false;
};
