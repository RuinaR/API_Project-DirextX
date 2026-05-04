#pragma once

#include "Component.h"

class CookieRunGameManagerComponent;
class ObjectPoolComponent;

class CookieRunObstacleComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunObstacleComponent";

	CookieRunObstacleComponent() = default;

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
	bool RequiresDoubleJump() const { return m_requiresDoubleJump; }

protected:
	void TriggerEnter(Collider2D* other) override;
	void TriggerStay(Collider2D* other) override;
	void CollisionEnter(Collider2D* other) override;
	void CollisionStay(Collider2D* other) override;

private:
	void ResolveFallbackReferences();
	void HandleHit(Collider2D* other);

	CookieRunGameManagerComponent* GetGameManager() const;
	ObjectPoolComponent* GetPool() const;

	Component* m_gameManagerComponent = nullptr;
	Component* m_poolComponent = nullptr;
	bool m_requiresDoubleJump = false;
	float m_hitCooldown = 1.0f;
	double m_lastHitTime = -999.0;
	bool m_disableAfterHit = false;
};
