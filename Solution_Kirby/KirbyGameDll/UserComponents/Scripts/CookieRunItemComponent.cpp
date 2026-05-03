#include "pch.h"
#include "CookieRunItemComponent.h"

#include "CookieRunGameManagerComponent.h"
#include "DebugLog.h"
#include "ImGuiContextBridge.h"
#include "ObjectManager.h"
#include "ObjectPoolComponent.h"
#include "SceneJsonUtility.h"
#include "SpawnObjectPoolComponent.h"

void CookieRunItemComponent::Initialize()
{
}

void CookieRunItemComponent::Release()
{
}

void CookieRunItemComponent::Start()
{
	ResolveFallbackReferences();
}

void CookieRunItemComponent::Update()
{
}

const char* CookieRunItemComponent::GetInspectorName() const
{
	return "CookieRun Item";
}

void CookieRunItemComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::DragInt("Score Value", &m_scoreValue, 1.0f, 1, 10000);
}

const char* CookieRunItemComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunItemComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"scoreValue\": " << m_scoreValue;
	oss << " }";
	return oss.str();
}

bool CookieRunItemComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadInt(componentJson, "scoreValue", m_scoreValue);
	return true;
}

void CookieRunItemComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterComponentRef("GameManager Component", &m_gameManagerComponent, "CookieRunGameManagerComponent");
	registry.RegisterComponentRef("ObjectPool Component", &m_poolComponent, "SpawnObjectPoolComponent");
}

void CookieRunItemComponent::TriggerEnter(Collider2D* other)
{
	HandleCollect(other);
}

void CookieRunItemComponent::CollisionEnter(Collider2D* other)
{
	HandleCollect(other);
}

void CookieRunItemComponent::ResolveFallbackReferences()
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

void CookieRunItemComponent::HandleCollect(Collider2D* other)
{
	if (m_collected || other == nullptr || other->GetGameObject() == nullptr)
	{
		return;
	}

	if (other->GetGameObject()->GetTag() != "Player")
	{
		return;
	}

	m_collected = true;
	if (CookieRunGameManagerComponent* gameManager = GetGameManager())
	{
		gameManager->AddScore(m_scoreValue);
	}

	if (ObjectPoolComponent* pool = GetPool())
	{
		pool->RecycleObject(GetGameObject());
	}
	else if (GetGameObject() != nullptr)
	{
		GetGameObject()->SetActive(false);
	}

	m_collected = false;
}

CookieRunGameManagerComponent* CookieRunItemComponent::GetGameManager() const
{
	return dynamic_cast<CookieRunGameManagerComponent*>(m_gameManagerComponent);
}

ObjectPoolComponent* CookieRunItemComponent::GetPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_poolComponent);
}
