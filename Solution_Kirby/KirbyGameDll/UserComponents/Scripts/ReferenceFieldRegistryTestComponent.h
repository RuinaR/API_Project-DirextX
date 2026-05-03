#pragma once

#include "Component.h"

class ReferenceFieldRegistryTestComponent : public Component
{
public:
	static constexpr const char* kComponentType = "ReferenceFieldRegistryTestComponent";

	ReferenceFieldRegistryTestComponent() = default;
	~ReferenceFieldRegistryTestComponent() override = default;

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
	void ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap) override;

private:
	std::string DescribeGameObject(class GameObject* gameObject) const;
	std::string DescribeComponent(class Component* component) const;

	bool m_enabled = true;

	GameObject* m_manualTargetObject = nullptr;
	int m_manualTargetObjectId = -1;

	GameObject* m_registryTargetObject = nullptr;
	Component* m_anyComponentRef = nullptr;
	Component* m_rigidbodyComponentRef = nullptr;
};
