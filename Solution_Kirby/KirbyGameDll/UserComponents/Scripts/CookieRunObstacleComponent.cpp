#include "pch.h"
#include "CookieRunObstacleComponent.h"

#include "CookieRunGameManagerComponent.h"
#include "DebugLog.h"
#include "ImGuiContextBridge.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "ObjectPoolComponent.h"
#include "SceneJsonUtility.h"
#include "SpawnObjectPoolComponent.h"

void CookieRunObstacleComponent::Initialize()
{
}

void CookieRunObstacleComponent::Release()
{
}

void CookieRunObstacleComponent::Start()
{
	ResolveFallbackReferences();
}

void CookieRunObstacleComponent::Update()
{
}

const char* CookieRunObstacleComponent::GetInspectorName() const
{
	return "CookieRun Obstacle";
}

void CookieRunObstacleComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::Checkbox("Requires Double Jump", &m_requiresDoubleJump);
	ImGui::DragFloat("Hit Cooldown", &m_hitCooldown, 0.05f, 0.0f, 10.0f);
	ImGui::Checkbox("Disable After Hit", &m_disableAfterHit);
}

const char* CookieRunObstacleComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunObstacleComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"requiresDoubleJump\": " << (m_requiresDoubleJump ? "true" : "false") << ", ";
	oss << "\"hitCooldown\": " << m_hitCooldown << ", ";
	oss << "\"disableAfterHit\": " << (m_disableAfterHit ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool CookieRunObstacleComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "requiresDoubleJump", m_requiresDoubleJump);
	SceneJson::ReadFloat(componentJson, "hitCooldown", m_hitCooldown);
	SceneJson::ReadBool(componentJson, "disableAfterHit", m_disableAfterHit);
	return true;
}

void CookieRunObstacleComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterComponentRef("GameManager Component", &m_gameManagerComponent, "CookieRunGameManagerComponent");
	registry.RegisterComponentRef("ObjectPool Component", &m_poolComponent, "SpawnObjectPoolComponent");
}

void CookieRunObstacleComponent::TriggerEnter(Collider2D* other)
{
	HandleHit(other);
}

void CookieRunObstacleComponent::CollisionEnter(Collider2D* other)
{
	HandleHit(other);
}

void CookieRunObstacleComponent::ResolveFallbackReferences()
{
	if (m_gameManagerComponent == nullptr)
	{
		if (GameObject* object = ObjectManager::GetInstance()->FindObjectByName("GameManager"))
		{
			m_gameManagerComponent = object->GetComponent<CookieRunGameManagerComponent>();
		}
	}

	if (m_poolComponent == nullptr)
	{
		GameObject* owner = GetGameObject();
		if (owner != nullptr && owner->GetParent() != nullptr)
		{
			m_poolComponent = owner->GetParent()->GetComponent<SpawnObjectPoolComponent>();
		}
	}
}

void CookieRunObstacleComponent::HandleHit(Collider2D* other)
{
	if (other == nullptr || other->GetGameObject() == nullptr)
	{
		return;
	}

	if (other->GetGameObject()->GetTag() != "Player")
	{
		return;
	}

	const double now = MainFrame::GetInstance()->Timer().getTotalDeltaTime();
	if (now - m_lastHitTime < static_cast<double>(m_hitCooldown))
	{
		return;
	}

	CookieRunGameManagerComponent* gameManager = GetGameManager();
	if (gameManager == nullptr)
	{
		return;
	}

	if (!gameManager->DamagePlayer(1))
	{
		return;
	}

	m_lastHitTime = now;
	DebugLog::Warning("Obstacle hit player.");

	if (ObjectPoolComponent* pool = GetPool())
	{
		pool->RecycleObject(GetGameObject());
	}
	else if (m_disableAfterHit && GetGameObject() != nullptr)
	{
		GetGameObject()->SetActive(false);
	}
}

CookieRunGameManagerComponent* CookieRunObstacleComponent::GetGameManager() const
{
	return dynamic_cast<CookieRunGameManagerComponent*>(m_gameManagerComponent);
}

ObjectPoolComponent* CookieRunObstacleComponent::GetPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_poolComponent);
}
