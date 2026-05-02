#pragma once

#include "Component.h"

class ActivePhysicsTestInfoComponent : public Component
{
public:
	static constexpr const char* kComponentType = "ActivePhysicsTestInfoComponent";

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap) override;

private:
	void RefreshLabel();

	GameObject* m_labelObject = nullptr;
	GameObject* m_playerObject = nullptr;
	int m_labelObjectId = -1;
	int m_playerObjectId = -1;
};
