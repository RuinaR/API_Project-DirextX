#include "pch.h"
#include "CookieRunPlayerControllerComponent.h"

#include "AnimationRender.h"
#include "CookieRunGameManagerComponent.h"
#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "Rigidbody2D.h"
#include "SceneJsonUtility.h"

void CookieRunPlayerControllerComponent::Initialize()
{
}

void CookieRunPlayerControllerComponent::Release()
{
}

void CookieRunPlayerControllerComponent::Start()
{
	ResolveFallbackReferences();
	RefreshAnimationState();
}

void CookieRunPlayerControllerComponent::Update()
{
	HandleJumpInput();
	ApplyAirMotionTuning();
	RefreshAnimationState();
}

const char* CookieRunPlayerControllerComponent::GetInspectorName() const
{
	return "CookieRun Player Controller";
}

void CookieRunPlayerControllerComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::DragInt("Max Jump Count", &m_maxJumpCount, 1.0f, 1, 5);
	ImGui::DragFloat("Jump Force", &m_jumpForce, 10.0f, 0.0f, 50000.0f);
	ImGui::DragFloat("Fall Accel", &m_fallAcceleration, 50.0f, 0.0f, 50000.0f);
	ImGui::DragFloat("Low Jump Accel", &m_lowJumpAcceleration, 50.0f, 0.0f, 50000.0f);
	ImGui::Text("Jump Count: %d", m_jumpCount);
	ImGui::Text("Grounded: %s", m_isGrounded ? "true" : "false");
}

const char* CookieRunPlayerControllerComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunPlayerControllerComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"maxJumpCount\": " << m_maxJumpCount << ", ";
	oss << "\"jumpForce\": " << m_jumpForce << ", ";
	oss << "\"fallAcceleration\": " << m_fallAcceleration << ", ";
	oss << "\"lowJumpAcceleration\": " << m_lowJumpAcceleration << ", ";
	oss << "\"idleAnimationKey\": \"" << SceneJson::EscapeString(m_idleAnimationKey) << "\", ";
	oss << "\"runAnimationKey\": \"" << SceneJson::EscapeString(m_runAnimationKey) << "\", ";
	oss << "\"jumpAnimationKey\": \"" << SceneJson::EscapeString(m_jumpAnimationKey) << "\"";
	oss << " }";
	return oss.str();
}

bool CookieRunPlayerControllerComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadInt(componentJson, "maxJumpCount", m_maxJumpCount);
	SceneJson::ReadFloat(componentJson, "jumpForce", m_jumpForce);
	SceneJson::ReadFloat(componentJson, "fallAcceleration", m_fallAcceleration);
	SceneJson::ReadFloat(componentJson, "lowJumpAcceleration", m_lowJumpAcceleration);
	SceneJson::ReadString(componentJson, "idleAnimationKey", m_idleAnimationKey);
	SceneJson::ReadString(componentJson, "runAnimationKey", m_runAnimationKey);
	SceneJson::ReadString(componentJson, "jumpAnimationKey", m_jumpAnimationKey);
	return true;
}

void CookieRunPlayerControllerComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterComponentRef("Rigidbody2D Component", &m_rigidbodyComponent, "Rigidbody2D");
	registry.RegisterComponentRef("AnimationRender Component", &m_animationRenderComponent, "AnimationRender");
	registry.RegisterComponentRef("GameManager Component", &m_gameManagerComponent, "CookieRunGameManagerComponent");
}

void CookieRunPlayerControllerComponent::CollisionEnter(Collider2D* other)
{
	EnterGround(other);
}

void CookieRunPlayerControllerComponent::CollisionStay(Collider2D* other)
{
	EnterGround(other);
}

void CookieRunPlayerControllerComponent::CollisionExit(Collider2D* other)
{
	LeaveGround(other);
}

void CookieRunPlayerControllerComponent::ResolveFallbackReferences()
{
	GameObject* owner = GetGameObject();
	if (owner != nullptr)
	{
		if (m_rigidbodyComponent == nullptr)
		{
			m_rigidbodyComponent = owner->GetComponent<Rigidbody2D>();
		}

		if (m_animationRenderComponent == nullptr)
		{
			m_animationRenderComponent = owner->GetComponent<AnimationRender>();
		}
	}

	if (m_gameManagerComponent == nullptr)
	{
		if (GameObject* gameManagerObject = ObjectManager::GetInstance()->FindObjectByName("GameManager"))
		{
			m_gameManagerComponent = gameManagerObject->GetComponent<CookieRunGameManagerComponent>();
		}
	}
}

void CookieRunPlayerControllerComponent::HandleJumpInput()
{
	const bool isSpaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

	if (CookieRunGameManagerComponent* gameManager = GetGameManager())
	{
		if (gameManager->IsScoreCollectEffectActive())
		{
			// 아이템 획득 연출 중에는 점프 입력을 잠깐 막는다.
			// 이때 눌린 Space도 현재 상태로 기억해서 연출이 끝난 직후 점프가 튀지 않게 한다.
			m_spaceWasDown = isSpaceDown;
			return;
		}
	}

	if (isSpaceDown && !m_spaceWasDown)
	{
		ApplyJump();
	}

	m_spaceWasDown = isSpaceDown;
}

void CookieRunPlayerControllerComponent::ApplyAirMotionTuning()
{
	Rigidbody2D* rigidbody = GetRigidbody();
	MainFrame* mainFrame = MainFrame::GetInstance();
	if (rigidbody == nullptr || mainFrame == nullptr || m_isGrounded)
	{
		return;
	}

	const float deltaTime = static_cast<float>(mainFrame->DeltaTime());
	if (deltaTime <= 0.0f)
	{
		return;
	}

	D3DXVECTOR2 velocity = rigidbody->GetVelocity();
	const bool isSpaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

	if (velocity.y < 0.0f)
	{
		velocity.y -= m_fallAcceleration * deltaTime;
		rigidbody->SetVelocity(velocity);
		return;
	}

	if (velocity.y > 0.0f && !isSpaceDown)
	{
		velocity.y -= m_lowJumpAcceleration * deltaTime;
		rigidbody->SetVelocity(velocity);
	}
}

void CookieRunPlayerControllerComponent::RefreshAnimationState()
{
	AnimationRender* animationRender = GetAnimationRender();
	Rigidbody2D* rigidbody = GetRigidbody();
	if (animationRender == nullptr || rigidbody == nullptr)
	{
		return;
	}

	std::string nextAnimationKey = m_runAnimationKey;
	if (!m_isGrounded || rigidbody->GetVelocity().y > 1.0f || rigidbody->GetVelocity().y < -1.0f)
	{
		nextAnimationKey = m_jumpAnimationKey;
	}

	if (m_currentAnimationKey == nextAnimationKey)
	{
		return;
	}

	m_currentAnimationKey = nextAnimationKey;
	animationRender->SetAnimationFolderKey(m_currentAnimationKey);
	animationRender->SetPlay(true);
}

void CookieRunPlayerControllerComponent::ApplyJump()
{
	Rigidbody2D* rigidbody = GetRigidbody();
	if (rigidbody == nullptr || m_jumpCount >= m_maxJumpCount)
	{
		return;
	}

	D3DXVECTOR2 velocity = rigidbody->GetVelocity();
	velocity.y = m_jumpForce;
	rigidbody->SetVelocity(velocity);
	++m_jumpCount;
	m_isGrounded = false;
	RefreshAnimationState();
}

void CookieRunPlayerControllerComponent::EnterGround(Collider2D* other)
{
	if (other == nullptr || other->GetGameObject() == nullptr)
	{
		return;
	}

	if (other->GetGameObject()->GetTag() != "Land")
	{
		return;
	}

	m_groundContactCount = max(m_groundContactCount, 1);
	m_isGrounded = true;
	m_jumpCount = 0;
}

void CookieRunPlayerControllerComponent::LeaveGround(Collider2D* other)
{
	if (other == nullptr || other->GetGameObject() == nullptr)
	{
		return;
	}

	if (other->GetGameObject()->GetTag() != "Land")
	{
		return;
	}

	m_groundContactCount = max(0, m_groundContactCount - 1);
	if (m_groundContactCount == 0)
	{
		m_isGrounded = false;
	}
}

Rigidbody2D* CookieRunPlayerControllerComponent::GetRigidbody() const
{
	return dynamic_cast<Rigidbody2D*>(m_rigidbodyComponent);
}

AnimationRender* CookieRunPlayerControllerComponent::GetAnimationRender() const
{
	return dynamic_cast<AnimationRender*>(m_animationRenderComponent);
}

CookieRunGameManagerComponent* CookieRunPlayerControllerComponent::GetGameManager() const
{
	return dynamic_cast<CookieRunGameManagerComponent*>(m_gameManagerComponent);
}
