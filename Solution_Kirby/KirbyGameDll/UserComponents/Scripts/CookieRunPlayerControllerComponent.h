#pragma once

#include <string>

#include "Component.h"

class AnimationRender;
class CookieRunGameManagerComponent;
class Rigidbody2D;

class CookieRunPlayerControllerComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunPlayerControllerComponent";

	CookieRunPlayerControllerComponent() = default;

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
	void CollisionEnter(Collider2D* other) override;
	void CollisionStay(Collider2D* other) override;
	void CollisionExit(Collider2D* other) override;

private:
	void ResolveFallbackReferences();
	void HandleJumpInput();
	void ApplyAirMotionTuning();
	void RefreshAnimationState();
	void ApplyJump();
	void EnterGround(Collider2D* other);
	void LeaveGround(Collider2D* other);

	Rigidbody2D* GetRigidbody() const;
	AnimationRender* GetAnimationRender() const;
	CookieRunGameManagerComponent* GetGameManager() const;

	Component* m_rigidbodyComponent = nullptr;
	Component* m_animationRenderComponent = nullptr;
	Component* m_gameManagerComponent = nullptr;
	int m_jumpCount = 0;
	int m_maxJumpCount = 2;
	int m_groundContactCount = 0;
	float m_jumpForce = 9000.0f;
	float m_fallAcceleration = 26000.0f;
	float m_lowJumpAcceleration = 18000.0f;
	bool m_isGrounded = false;
	std::string m_idleAnimationKey = "Animation/Player/default/right/idle.anim";
	std::string m_runAnimationKey = "Animation/Player/default/right/run.anim";
	std::string m_jumpAnimationKey = "Animation/Player/default/right/jump.anim";
	std::string m_currentAnimationKey;
};
