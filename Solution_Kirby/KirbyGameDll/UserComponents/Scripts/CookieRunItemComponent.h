#pragma once

#include "Component.h"

class CookieRunGameManagerComponent;
class ObjectPoolComponent;

class CookieRunItemComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunItemComponent";

	CookieRunItemComponent() = default;

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

protected:
	void TriggerEnter(Collider2D* other) override;
	void CollisionEnter(Collider2D* other) override;

private:
	void ResolveFallbackReferences();
	void HandleCollect(Collider2D* other);

	CookieRunGameManagerComponent* GetGameManager() const;
	ObjectPoolComponent* GetPool() const;

	Component* m_gameManagerComponent = nullptr;
	Component* m_poolComponent = nullptr;
	int m_scoreValue = 10;
	bool m_collected = false;
};
