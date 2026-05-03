#pragma once

#include "Component.h"

enum class Rigidbody2DCollisionDetection
{
	Discrete = 0,
	Continuous,
};

enum class Rigidbody2DInterpolation
{
	None = 0,
	Interpolate,
	Extrapolate,
};

class ENGINEFRAMEWORK_API Rigidbody2D : public Component
{
public:
	Rigidbody2D();
	~Rigidbody2D() override = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void CreateBody(bool rebuildAttachedColliders = true);
	void ApplyBodySettings();
	void RebuildAttachedColliders();
	void SetPhysicsActive(bool active);
	void SyncBodyToGameObjectTransform();

	// body가 없으면 캐시만 갱신한다.
	// static body면 캐시만 갱신하고, kinematic/dynamic body면 runtime body에도 적용한다.
	void SetVelocity(const D3DXVECTOR2& velocity);
	// body가 있으면 runtime body velocity를 반환하고, 없으면 캐시 값을 반환한다.
	D3DXVECTOR2 GetVelocity() const;
	// body가 없으면 캐시만 갱신한다.
	// static body면 캐시만 갱신하고, kinematic/dynamic body면 runtime body에도 적용한다.
	void SetAngularVelocity(float angularVelocity);
	// body가 있으면 runtime body angular velocity를 반환하고, 없으면 캐시 값을 반환한다.
	float GetAngularVelocity() const;
	// force/impulse/torque 계열은 dynamic body에서만 실제 적용된다.
	// static/kinematic/body 없음 상태에서는 조용히 무시한다.
	void AddForce(const D3DXVECTOR2& force);
	void AddImpulse(const D3DXVECTOR2& impulse);
	void AddTorque(float torque);
	b2Body* GetBody() const;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

private:
	b2BodyType m_bodyType = b2_staticBody;
	float m_drag = 0.0f;
	float m_angularDrag = 0.0f;
	bool m_useGravity = true;
	float m_gravityScale = 1.0f;
	bool m_fixedRotation = false;
	D3DXVECTOR2 m_linearVelocity = D3DXVECTOR2(0.0f, 0.0f);
	float m_angularVelocity = 0.0f;
	Rigidbody2DCollisionDetection m_collisionDetection = Rigidbody2DCollisionDetection::Discrete;
	Rigidbody2DInterpolation m_interpolate = Rigidbody2DInterpolation::None;
	bool m_freezeRotationZ = false;
	b2Body* m_body = nullptr;
};
