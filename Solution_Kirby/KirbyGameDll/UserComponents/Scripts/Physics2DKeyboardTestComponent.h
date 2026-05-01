#pragma once

#include "Component.h"

class Rigidbody2D;
class ImageRender;

class Physics2DKeyboardTestComponent : public Component
{
public:
	static constexpr const char* kComponentType = "Physics2DKeyboardTestComponent";

	Physics2DKeyboardTestComponent() = default;
	~Physics2DKeyboardTestComponent() override = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

protected:
	void CollisionEnter(Collider2D* other) override;
	void CollisionStay(Collider2D* other) override;
	void CollisionExit(Collider2D* other) override;
	void TriggerEnter(Collider2D* other) override;
	void TriggerStay(Collider2D* other) override;
	void TriggerExit(Collider2D* other) override;

private:
	Rigidbody2D* GetRigidbody2D() const;
	ImageRender* GetImageRender() const;
	void CacheStartTransform();
	void CacheOriginalColor();
	void HandleHorizontalInput(Rigidbody2D* rigidbody2D);
	void HandleJumpInput(Rigidbody2D* rigidbody2D, bool jumpPressed);
	void HandleRotationInput(Rigidbody2D* rigidbody2D, float rotationAxis);
	void HandleResetInput(Rigidbody2D* rigidbody2D, bool resetPressed);
	void RefreshVisualState();

	bool m_enabled = true;
	float m_forcePower = 220.0f;
	float m_impulsePower = 90000.0f;
	float m_torquePower = 6.0f;
	bool m_useVelocityMode = true;

	D3DXVECTOR3 m_startPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float m_startAngleZ = 0.0f;
	bool m_hasStartTransform = false;
	D3DCOLOR m_originalColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	bool m_hasOriginalColor = false;
	int m_activeCollisionCount = 0;
	int m_activeTriggerCount = 0;
	bool m_prevJumpPressed = false;
	bool m_prevResetPressed = false;
};
