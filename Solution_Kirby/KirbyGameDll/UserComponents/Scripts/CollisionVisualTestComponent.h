#pragma once

#include "Component.h"

class CollisionVisualTestComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CollisionVisualTestComponent";
	enum class MouseVisualEvent
	{
		None = 0,
		LeftDown,
		LeftUp,
		RightDown,
		RightUp,
	};

	CollisionVisualTestComponent() = default;
	~CollisionVisualTestComponent() override = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void OnLBtnDown() override;
	void OnLBtnUp() override;
	void OnRBtnDown() override;
	void OnRBtnUp() override;

protected:
	void CollisionEnter(Collider* other) override;
	void CollisionStay(Collider* other) override;
	void CollisionExit(Collider* other) override;

private:
	void CacheOriginalSize();
	D3DXVECTOR2 ReadMoveInput() const;
	void CacheOriginalColor();
	void RefreshVisualState();
	void TriggerMouseVisual(MouseVisualEvent visualEvent);
	D3DCOLOR GetMouseVisualColor() const;
	D3DXVECTOR2 GetMouseVisualScale() const;
	const char* GetMouseVisualLabel() const;

	bool m_enabled = true;
	float m_scaleOnEnter = 1.2f;
	float m_moveSpeed = 250.0f;
	D3DXVECTOR3 m_originalSize = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	bool m_hasOriginalSize = false;
	D3DCOLOR m_originalColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	bool m_hasOriginalColor = false;
	int m_activeCollisionCount = 0;
	float m_visualTime = 0.0f;
	float m_mouseVisualTimer = 0.0f;
	MouseVisualEvent m_mouseVisualEvent = MouseVisualEvent::None;
};
