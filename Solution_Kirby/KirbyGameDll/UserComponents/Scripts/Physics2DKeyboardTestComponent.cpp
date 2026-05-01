#include "pch.h"
#include "Physics2DKeyboardTestComponent.h"

#include "GameObject.h"
#include "ImageRender.h"
#include "Rigidbody2D.h"
#include "SceneJsonUtility.h"

namespace
{
	float ReadHorizontalAxis()
	{
		float axis = 0.0f;
		if ((GetAsyncKeyState('A') & 0x8000) != 0 || (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0)
		{
			axis -= 1.0f;
		}
		if ((GetAsyncKeyState('D') & 0x8000) != 0 || (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0)
		{
			axis += 1.0f;
		}
		return axis;
	}

	float ReadRotationAxis()
	{
		float axis = 0.0f;
		if ((GetAsyncKeyState('Q') & 0x8000) != 0)
		{
			axis -= 1.0f;
		}
		if ((GetAsyncKeyState('E') & 0x8000) != 0)
		{
			axis += 1.0f;
		}
		return axis;
	}
}

void Physics2DKeyboardTestComponent::Initialize()
{
	CacheStartTransform();
	CacheOriginalColor();
}

void Physics2DKeyboardTestComponent::Release()
{
	if (ImageRender* imageRender = GetImageRender())
	{
		if (m_hasOriginalColor)
		{
			imageRender->SetColor(m_originalColor);
		}
	}

	m_activeCollisionCount = 0;
	m_activeTriggerCount = 0;
	m_prevJumpPressed = false;
	m_prevResetPressed = false;
}

void Physics2DKeyboardTestComponent::Start()
{
	CacheStartTransform();
	CacheOriginalColor();
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::Update()
{
	if (!m_enabled || m_gameObj == nullptr)
	{
		return;
	}

	Rigidbody2D* rigidbody2D = GetRigidbody2D();
	if (rigidbody2D == nullptr)
	{
		return;
	}

	HandleHorizontalInput(rigidbody2D);

	const bool jumpPressed =
		((GetAsyncKeyState('W') & 0x8000) != 0) ||
		((GetAsyncKeyState(VK_UP) & 0x8000) != 0) ||
		((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0);
	HandleJumpInput(rigidbody2D, jumpPressed);

	HandleRotationInput(rigidbody2D, ReadRotationAxis());

	const bool resetPressed = (GetAsyncKeyState('R') & 0x8000) != 0;
	HandleResetInput(rigidbody2D, resetPressed);
	RefreshVisualState();
}

const char* Physics2DKeyboardTestComponent::GetInspectorName() const
{
	return "Physics2D Keyboard Test";
}

void Physics2DKeyboardTestComponent::DrawInspector()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::DragFloat("Force Power", &m_forcePower, 10.0f, 0.0f, 100000.0f, "%.1f");
	ImGui::DragFloat("Impulse Power", &m_impulsePower, 10.0f, 0.0f, 100000.0f, "%.1f");
	ImGui::DragFloat("Torque Power", &m_torquePower, 1.0f, 0.0f, 100000.0f, "%.1f");
	ImGui::Checkbox("Use Velocity Mode", &m_useVelocityMode);
	ImGui::TextDisabled("A/D move, W/Up/Space jump, Q/E rotate, R reset");
	ImGui::TextDisabled("Default test values are tuned for this project's large Box2D unit scale.");
	ImGui::Text("Collision Contacts: %d", m_activeCollisionCount);
	ImGui::Text("Trigger Contacts: %d", m_activeTriggerCount);

	if (GetRigidbody2D() == nullptr)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "Rigidbody 2D is required.");
	}
}

const char* Physics2DKeyboardTestComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string Physics2DKeyboardTestComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"forcePower\": " << m_forcePower << ", ";
	oss << "\"impulsePower\": " << m_impulsePower << ", ";
	oss << "\"torquePower\": " << m_torquePower << ", ";
	oss << "\"useVelocityMode\": " << (m_useVelocityMode ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool Physics2DKeyboardTestComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "enabled", m_enabled);
	SceneJson::ReadFloat(componentJson, "forcePower", m_forcePower);
	SceneJson::ReadFloat(componentJson, "impulsePower", m_impulsePower);
	SceneJson::ReadFloat(componentJson, "torquePower", m_torquePower);
	SceneJson::ReadBool(componentJson, "useVelocityMode", m_useVelocityMode);
	return true;
}

Rigidbody2D* Physics2DKeyboardTestComponent::GetRigidbody2D() const
{
	return m_gameObj != nullptr ? m_gameObj->GetComponent<Rigidbody2D>() : nullptr;
}

ImageRender* Physics2DKeyboardTestComponent::GetImageRender() const
{
	return m_gameObj != nullptr ? m_gameObj->GetComponent<ImageRender>() : nullptr;
}

void Physics2DKeyboardTestComponent::CacheStartTransform()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	m_startPosition = m_gameObj->Position();
	m_startAngleZ = m_gameObj->GetAngleZ();
	m_hasStartTransform = true;
}

void Physics2DKeyboardTestComponent::CacheOriginalColor()
{
	if (ImageRender* imageRender = GetImageRender())
	{
		m_originalColor = imageRender->GetColor();
		m_hasOriginalColor = true;
	}
}

void Physics2DKeyboardTestComponent::HandleHorizontalInput(Rigidbody2D* rigidbody2D)
{
	const float axis = ReadHorizontalAxis();
	if (m_useVelocityMode)
	{
		const D3DXVECTOR2 currentVelocity = rigidbody2D->GetVelocity();
		rigidbody2D->SetVelocity(D3DXVECTOR2(axis * m_forcePower, currentVelocity.y));
		return;
	}

	if (axis != 0.0f)
	{
		rigidbody2D->AddForce(D3DXVECTOR2(axis * m_forcePower, 0.0f));
	}
}

void Physics2DKeyboardTestComponent::HandleJumpInput(Rigidbody2D* rigidbody2D, bool jumpPressed)
{
	if (jumpPressed && !m_prevJumpPressed)
	{
		rigidbody2D->AddImpulse(D3DXVECTOR2(0.0f, m_impulsePower));
	}

	m_prevJumpPressed = jumpPressed;
}

void Physics2DKeyboardTestComponent::HandleRotationInput(Rigidbody2D* rigidbody2D, float rotationAxis)
{
	if (m_useVelocityMode)
	{
		rigidbody2D->SetAngularVelocity(rotationAxis * m_torquePower);
		return;
	}

	if (rotationAxis != 0.0f)
	{
		rigidbody2D->AddTorque(rotationAxis * m_torquePower);
	}
}

void Physics2DKeyboardTestComponent::HandleResetInput(Rigidbody2D* rigidbody2D, bool resetPressed)
{
	if (resetPressed && !m_prevResetPressed && m_gameObj != nullptr && m_hasStartTransform)
	{
		m_gameObj->SetPosition(m_startPosition);
		m_gameObj->SetAngleZ(m_startAngleZ);
		rigidbody2D->SetVelocity(D3DXVECTOR2(0.0f, 0.0f));
		rigidbody2D->SetAngularVelocity(0.0f);
	}

	m_prevResetPressed = resetPressed;
}

void Physics2DKeyboardTestComponent::RefreshVisualState()
{
	ImageRender* imageRender = GetImageRender();
	if (imageRender == nullptr || !m_hasOriginalColor)
	{
		return;
	}

	if (m_activeCollisionCount > 0 && m_activeTriggerCount > 0)
	{
		imageRender->SetColor(D3DCOLOR_ARGB(255, 255, 140, 70));
		return;
	}

	if (m_activeCollisionCount > 0)
	{
		imageRender->SetColor(D3DCOLOR_ARGB(255, 255, 90, 90));
		return;
	}

	if (m_activeTriggerCount > 0)
	{
		imageRender->SetColor(D3DCOLOR_ARGB(255, 80, 220, 255));
		return;
	}

	imageRender->SetColor(m_originalColor);
}

void Physics2DKeyboardTestComponent::CollisionEnter(Collider2D* other)
{
	(void)other;
	++m_activeCollisionCount;
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::CollisionStay(Collider2D* other)
{
	(void)other;
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::CollisionExit(Collider2D* other)
{
	(void)other;
	if (m_activeCollisionCount > 0)
	{
		--m_activeCollisionCount;
	}
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::TriggerEnter(Collider2D* other)
{
	(void)other;
	++m_activeTriggerCount;
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::TriggerStay(Collider2D* other)
{
	(void)other;
	RefreshVisualState();
}

void Physics2DKeyboardTestComponent::TriggerExit(Collider2D* other)
{
	(void)other;
	if (m_activeTriggerCount > 0)
	{
		--m_activeTriggerCount;
	}
	RefreshVisualState();
}
