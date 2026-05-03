#pragma once

#include "Component.h"

class ObjectManagerSearchApiTestComponent : public Component
{
public:
	static constexpr const char* kComponentType = "ObjectManagerSearchApiTestComponent";

	ObjectManagerSearchApiTestComponent() = default;
	~ObjectManagerSearchApiTestComponent() override = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

private:
	void SetupTestObjects();
	void RunSearchSnapshot(const char* label);
	void RunLegacySearchSnapshot(const char* label);
	void RunDestroyFirstByTag();
	void RunDestroyAllByTag();
	void RunDestroyByName();
	void PushLog(const std::string& text);
	void MarkCurrentSceneDirty();
	std::string DescribeObject(class GameObject* obj) const;

	bool m_enabled = true;
	std::string m_testTag = "ObjectManagerApiTest";
	int m_spawnCount = 3;
	float m_spacingX = 120.0f;
	std::vector<std::string> m_recentLogs;
};
