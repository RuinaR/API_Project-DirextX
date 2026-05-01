#include "pch.h"
#include "CollisionVisualTestComponent.h"

#include "GameObject.h"
#include "ImageRender.h"
#include "MainFrame.h"
#include "SceneJsonUtility.h"

namespace
{
	float ClampScaleMultiplier(float value)
	{
		return value < 1.0f ? 1.0f : value;
	}

	constexpr float kMouseVisualDuration = 0.18f;
	constexpr float kHoverTransitionDuration = 0.12f;
}

void CollisionVisualTestComponent::Initialize()
{
	CacheOriginalSize();
	CacheOriginalColor();
}

void CollisionVisualTestComponent::Release()
{
	if (m_gameObj != nullptr && m_hasOriginalSize)
	{
		m_gameObj->Size3D() = m_originalSize;
	}
	if (m_gameObj != nullptr && m_hasOriginalColor)
	{
		if (ImageRender* imageRender = m_gameObj->GetComponent<ImageRender>())
		{
			imageRender->SetColor(m_originalColor);
		}
	}

	m_activeCollisionCount = 0;
	m_visualTime = 0.0f;
	m_mouseVisualTimer = 0.0f;
	m_hoverTransitionTimer = 0.0f;
	m_isHovering = false;
	m_mouseVisualEvent = MouseVisualEvent::None;
	m_hoverVisualState = HoverVisualState::None;
}

void CollisionVisualTestComponent::Start()
{
	CacheOriginalSize();
	CacheOriginalColor();
}

void CollisionVisualTestComponent::Update()
{
	if (!m_enabled || m_gameObj == nullptr)
	{
		return;
	}

	const float deltaTime = MainFrame::GetInstance() != nullptr
		? static_cast<float>(MainFrame::GetInstance()->DeltaTime())
		: 0.0f;
	if (deltaTime <= 0.0f)
	{
		return;
	}

	const D3DXVECTOR2 moveInput = ReadMoveInput();
	if (moveInput.x != 0.0f || moveInput.y != 0.0f)
	{
		m_gameObj->AddPosition(D3DXVECTOR3(
			moveInput.x * m_moveSpeed * deltaTime,
			moveInput.y * m_moveSpeed * deltaTime,
			0.0f));
	}

	if (m_activeCollisionCount > 0)
	{
		m_visualTime += deltaTime;
		m_gameObj->SetAngleZ(m_gameObj->GetAngleZ() + (120.0f * deltaTime));
	}

	if (m_mouseVisualTimer > 0.0f)
	{
		m_mouseVisualTimer -= deltaTime;
		if (m_mouseVisualTimer <= 0.0f)
		{
			m_mouseVisualTimer = 0.0f;
			m_mouseVisualEvent = MouseVisualEvent::None;
		}
	}

	if (m_hoverTransitionTimer > 0.0f)
	{
		m_hoverTransitionTimer -= deltaTime;
		if (m_hoverTransitionTimer <= 0.0f)
		{
			m_hoverTransitionTimer = 0.0f;
			if (!m_isHovering && m_hoverVisualState == HoverVisualState::Exit)
			{
				m_hoverVisualState = HoverVisualState::None;
			}
		}
	}

	RefreshVisualState();
}

const char* CollisionVisualTestComponent::GetInspectorName() const
{
	return "Collision Visual Test";
}

void CollisionVisualTestComponent::DrawInspector()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::DragFloat("Scale On Enter", &m_scaleOnEnter, 0.01f, 1.0f, 3.0f, "%.2f");
	m_scaleOnEnter = ClampScaleMultiplier(m_scaleOnEnter);
	ImGui::Text("Hover State: %s", GetHoverVisualLabel());
	ImGui::Text("Last Mouse Event: %s", GetMouseVisualLabel());
}

const char* CollisionVisualTestComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CollisionVisualTestComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"scaleOnEnter\": " << m_scaleOnEnter;
	oss << " }";
	return oss.str();
}

bool CollisionVisualTestComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "enabled", m_enabled);
	SceneJson::ReadFloat(componentJson, "scaleOnEnter", m_scaleOnEnter);
	m_scaleOnEnter = ClampScaleMultiplier(m_scaleOnEnter);
	return true;
}

void CollisionVisualTestComponent::LBtnDown()
{
	if (!m_enabled)
	{
		return;
	}

	TriggerMouseVisual(MouseVisualEvent::LeftDown);
}

void CollisionVisualTestComponent::LBtnUp()
{
	if (!m_enabled)
	{
		return;
	}

	TriggerMouseVisual(MouseVisualEvent::LeftUp);
}

void CollisionVisualTestComponent::RBtnDown()
{
	if (!m_enabled)
	{
		return;
	}

	TriggerMouseVisual(MouseVisualEvent::RightDown);
}

void CollisionVisualTestComponent::RBtnUp()
{
	if (!m_enabled)
	{
		return;
	}

	TriggerMouseVisual(MouseVisualEvent::RightUp);
}

void CollisionVisualTestComponent::MouseHoverEnter()
{
	if (!m_enabled)
	{
		return;
	}

	m_isHovering = true;
	m_hoverVisualState = HoverVisualState::Enter;
	m_hoverTransitionTimer = kHoverTransitionDuration;
	RefreshVisualState();
}

void CollisionVisualTestComponent::MouseHoverStay()
{
	if (!m_enabled)
	{
		return;
	}

	m_isHovering = true;
	if (m_hoverVisualState != HoverVisualState::Enter || m_hoverTransitionTimer <= 0.0f)
	{
		m_hoverVisualState = HoverVisualState::Stay;
	}
	RefreshVisualState();
}

void CollisionVisualTestComponent::MouseHoverExit()
{
	if (!m_enabled)
	{
		return;
	}

	m_isHovering = false;
	m_hoverVisualState = HoverVisualState::Exit;
	m_hoverTransitionTimer = kHoverTransitionDuration;
	RefreshVisualState();
}

void CollisionVisualTestComponent::CollisionEnter(Collider* other)
{
	(void)other;

	if (!m_enabled || m_gameObj == nullptr)
	{
		return;
	}

	CacheOriginalSize();
	CacheOriginalColor();
	m_activeCollisionCount++;
	RefreshVisualState();
}

void CollisionVisualTestComponent::CollisionStay(Collider* other)
{
	(void)other;

	if (!m_enabled || m_gameObj == nullptr || m_activeCollisionCount <= 0)
	{
		return;
	}

	RefreshVisualState();
}

void CollisionVisualTestComponent::CollisionExit(Collider* other)
{
	(void)other;

	if (m_activeCollisionCount > 0)
	{
		m_activeCollisionCount--;
	}

	if (m_activeCollisionCount <= 0)
	{
		m_visualTime = 0.0f;
	}

	RefreshVisualState();
}

void CollisionVisualTestComponent::CacheOriginalSize()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	m_originalSize = m_gameObj->Size3D();
	m_hasOriginalSize = true;
}

D3DXVECTOR2 CollisionVisualTestComponent::ReadMoveInput() const
{
	D3DXVECTOR2 moveInput(0.0f, 0.0f);

	if ((GetAsyncKeyState('A') & 0x8000) != 0 || (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0)
	{
		moveInput.x -= 1.0f;
	}
	if ((GetAsyncKeyState('D') & 0x8000) != 0 || (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0)
	{
		moveInput.x += 1.0f;
	}
	if ((GetAsyncKeyState('W') & 0x8000) != 0 || (GetAsyncKeyState(VK_UP) & 0x8000) != 0)
	{
		moveInput.y += 1.0f;
	}
	if ((GetAsyncKeyState('S') & 0x8000) != 0 || (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0)
	{
		moveInput.y -= 1.0f;
	}

	const float lengthSq = (moveInput.x * moveInput.x) + (moveInput.y * moveInput.y);
	if (lengthSq > 1.0f)
	{
		const float invLength = 1.0f / sqrtf(lengthSq);
		moveInput.x *= invLength;
		moveInput.y *= invLength;
	}

	return moveInput;
}

void CollisionVisualTestComponent::CacheOriginalColor()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	if (ImageRender* imageRender = m_gameObj->GetComponent<ImageRender>())
	{
		m_originalColor = imageRender->GetColor();
		m_hasOriginalColor = true;
	}
}

void CollisionVisualTestComponent::RefreshVisualState()
{
	if (m_gameObj == nullptr || !m_hasOriginalSize)
	{
		return;
	}

	float collisionScale = 1.0f;
	if (m_activeCollisionCount > 0)
	{
		collisionScale = ClampScaleMultiplier(m_scaleOnEnter) * (1.0f + (sinf(m_visualTime * 8.0f) * 0.08f));
	}

	D3DXVECTOR3 scaledSize = m_originalSize;
	scaledSize.x *= collisionScale;
	scaledSize.y *= collisionScale;

	if (m_hoverVisualState == HoverVisualState::Enter && m_hoverTransitionTimer > 0.0f)
	{
		scaledSize.x *= 1.10f;
		scaledSize.y *= 1.10f;
	}
	else if (IsHoverStayActive())
	{
		scaledSize.x *= 1.04f;
		scaledSize.y *= 1.04f;
	}
	else if (m_hoverVisualState == HoverVisualState::Exit && m_hoverTransitionTimer > 0.0f)
	{
		scaledSize.x *= 0.96f;
		scaledSize.y *= 0.96f;
	}

	const D3DXVECTOR2 mouseScale = GetMouseVisualScale();
	scaledSize.x *= mouseScale.x;
	scaledSize.y *= mouseScale.y;
	m_gameObj->Size3D() = scaledSize;

	if (ImageRender* imageRender = m_gameObj->GetComponent<ImageRender>())
	{
		if (m_mouseVisualEvent != MouseVisualEvent::None && m_mouseVisualTimer > 0.0f)
		{
			imageRender->SetColor(GetMouseVisualColor());
		}
		else if (m_hoverVisualState == HoverVisualState::Enter && m_hoverTransitionTimer > 0.0f)
		{
			imageRender->SetColor(D3DCOLOR_ARGB(255, 120, 255, 180));
		}
		else if (IsHoverStayActive())
		{
			imageRender->SetColor(D3DCOLOR_ARGB(255, 150, 210, 255));
		}
		else if (m_hoverVisualState == HoverVisualState::Exit && m_hoverTransitionTimer > 0.0f)
		{
			imageRender->SetColor(D3DCOLOR_ARGB(255, 255, 170, 120));
		}
		else if (m_hasOriginalColor)
		{
			imageRender->SetColor(m_originalColor);
		}
	}
}

void CollisionVisualTestComponent::TriggerMouseVisual(MouseVisualEvent visualEvent)
{
	CacheOriginalSize();
	CacheOriginalColor();
	m_mouseVisualEvent = visualEvent;
	m_mouseVisualTimer = kMouseVisualDuration;
	RefreshVisualState();
}

D3DCOLOR CollisionVisualTestComponent::GetMouseVisualColor() const
{
	switch (m_mouseVisualEvent)
	{
	case MouseVisualEvent::LeftDown:
		return D3DCOLOR_ARGB(255, 255, 220, 80);
	case MouseVisualEvent::LeftUp:
		return D3DCOLOR_ARGB(255, 80, 255, 140);
	case MouseVisualEvent::RightDown:
		return D3DCOLOR_ARGB(255, 255, 90, 90);
	case MouseVisualEvent::RightUp:
		return D3DCOLOR_ARGB(255, 90, 220, 255);
	default:
		return m_originalColor;
	}
}

D3DXVECTOR2 CollisionVisualTestComponent::GetMouseVisualScale() const
{
	switch (m_mouseVisualEvent)
	{
	case MouseVisualEvent::LeftDown:
		return D3DXVECTOR2(1.18f, 0.92f);
	case MouseVisualEvent::LeftUp:
		return D3DXVECTOR2(0.92f, 1.18f);
	case MouseVisualEvent::RightDown:
		return D3DXVECTOR2(0.85f, 0.85f);
	case MouseVisualEvent::RightUp:
		return D3DXVECTOR2(1.12f, 1.12f);
	default:
		return D3DXVECTOR2(1.0f, 1.0f);
	}
}

const char* CollisionVisualTestComponent::GetMouseVisualLabel() const
{
	switch (m_mouseVisualEvent)
	{
	case MouseVisualEvent::LeftDown:
		return "Left Down";
	case MouseVisualEvent::LeftUp:
		return "Left Up";
	case MouseVisualEvent::RightDown:
		return "Right Down";
	case MouseVisualEvent::RightUp:
		return "Right Up";
	default:
		return "None";
	}
}

const char* CollisionVisualTestComponent::GetHoverVisualLabel() const
{
	switch (m_hoverVisualState)
	{
	case HoverVisualState::Enter:
		return "Enter";
	case HoverVisualState::Stay:
		return "Stay";
	case HoverVisualState::Exit:
		return "Exit";
	default:
		return "None";
	}
}

bool CollisionVisualTestComponent::IsHoverStayActive() const
{
	return m_isHovering && (m_hoverVisualState == HoverVisualState::Stay ||
		(m_hoverVisualState == HoverVisualState::Enter && m_hoverTransitionTimer <= 0.0f));
}
