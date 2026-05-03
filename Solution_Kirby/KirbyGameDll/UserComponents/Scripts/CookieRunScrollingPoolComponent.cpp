#include "pch.h"
#include "CookieRunScrollingPoolComponent.h"

#include "CookieRunGameManagerComponent.h"
#include "ImGuiContextBridge.h"
#include "LoopObjectPoolComponent.h"
#include "ObjectPoolComponent.h"
#include "ObjectManager.h"
#include "SceneJsonUtility.h"
#include "SpawnObjectPoolComponent.h"

void CookieRunScrollingPoolComponent::Initialize()
{
}

void CookieRunScrollingPoolComponent::Release()
{
}

void CookieRunScrollingPoolComponent::Start()
{
	ResolveFallbackReferences();
}

void CookieRunScrollingPoolComponent::Update()
{
	ObjectPoolComponent* pool = GetPool();
	if (pool == nullptr)
	{
		return;
	}

	const float deltaTime = static_cast<float>(MainFrame::GetInstance()->DeltaTime());
	const CookieRunGameManagerComponent* gameManager = GetGameManager();
	const float scrollSpeed = (m_useGameManagerSpeed && gameManager != nullptr)
		? gameManager->GetScrollSpeed()
		: m_scrollSpeed;

	const std::vector<GameObject*>& poolObjects = pool->GetPoolObjects();
	float rightMostEdge = -FLT_MAX;
	for (std::vector<GameObject*>::const_iterator itr = poolObjects.begin(); itr != poolObjects.end(); ++itr)
	{
		GameObject* object = *itr;
		if (object == nullptr || !object->GetActive())
		{
			continue;
		}

		D3DXVECTOR3 position = object->Position();
		position.x -= scrollSpeed * deltaTime;
		object->SetPosition(position);
		rightMostEdge = max(rightMostEdge, position.x + object->Size2D().x * 0.5f);

		const float halfWidth = object->Size2D().x * 0.5f;
		if (position.x + halfWidth < m_leftBound)
		{
			pool->RecycleObject(object);
		}
	}

	if (!pool->GetRepositionInsteadOfDisable())
	{
		// Spawn Pool은 오른쪽 공간이 비면 비활성 오브젝트를 다시 켠다.
		while (rightMostEdge < pool->GetRightSpawnX())
		{
			GameObject* activatedObject = pool->ActivateNextInactiveObject();
			if (activatedObject == nullptr)
			{
				break;
			}

			rightMostEdge = max(rightMostEdge, activatedObject->Position().x + activatedObject->Size2D().x * 0.5f);
		}
	}
}

const char* CookieRunScrollingPoolComponent::GetInspectorName() const
{
	return "CookieRun Scrolling Pool";
}

void CookieRunScrollingPoolComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::DragFloat("Scroll Speed", &m_scrollSpeed, 1.0f, 0.0f, 5000.0f);
	ImGui::DragFloat("Left Bound", &m_leftBound, 1.0f, -5000.0f, 5000.0f);
	ImGui::Checkbox("Use GameManager Speed", &m_useGameManagerSpeed);
}

const char* CookieRunScrollingPoolComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunScrollingPoolComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"scrollSpeed\": " << m_scrollSpeed << ", ";
	oss << "\"leftBound\": " << m_leftBound << ", ";
	oss << "\"useGameManagerSpeed\": " << (m_useGameManagerSpeed ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool CookieRunScrollingPoolComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadFloat(componentJson, "scrollSpeed", m_scrollSpeed);
	SceneJson::ReadFloat(componentJson, "leftBound", m_leftBound);
	SceneJson::ReadBool(componentJson, "useGameManagerSpeed", m_useGameManagerSpeed);
	return true;
}

void CookieRunScrollingPoolComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterComponentRef("ObjectPool Component", &m_poolComponent);
	registry.RegisterComponentRef("GameManager Component", &m_gameManagerComponent, "CookieRunGameManagerComponent");
}

void CookieRunScrollingPoolComponent::ResolveFallbackReferences()
{
	GameObject* owner = GetGameObject();
	if (owner != nullptr && m_poolComponent == nullptr)
	{
		m_poolComponent = owner->GetComponent<LoopObjectPoolComponent>();
		if (m_poolComponent == nullptr)
		{
			m_poolComponent = owner->GetComponent<SpawnObjectPoolComponent>();
		}
	}

	if (m_gameManagerComponent == nullptr)
	{
		if (GameObject* object = ObjectManager::GetInstance()->FindObjectByName("GameManager"))
		{
			m_gameManagerComponent = object->GetComponent<CookieRunGameManagerComponent>();
		}
	}
}

ObjectPoolComponent* CookieRunScrollingPoolComponent::GetPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_poolComponent);
}

CookieRunGameManagerComponent* CookieRunScrollingPoolComponent::GetGameManager() const
{
	return dynamic_cast<CookieRunGameManagerComponent*>(m_gameManagerComponent);
}
