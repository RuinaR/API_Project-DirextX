#include "pch.h"
#include "ObjectPoolComponent.h"

#include <algorithm>

#include "CookieRunObstacleComponent.h"
#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "SceneJsonUtility.h"

namespace
{
	GameObject* FindDoubleJumpPartner(GameObject* sourceObject, const std::vector<GameObject*>& poolObjects, float sourceX)
	{
		if (sourceObject == nullptr)
		{
			return nullptr;
		}

		CookieRunObstacleComponent* sourceObstacle = sourceObject->GetComponent<CookieRunObstacleComponent>();
		if (sourceObstacle == nullptr || !sourceObstacle->RequiresDoubleJump())
		{
			return nullptr;
		}

		for (std::vector<GameObject*>::const_iterator itr = poolObjects.begin(); itr != poolObjects.end(); ++itr)
		{
			GameObject* candidate = *itr;
			if (candidate == nullptr || candidate == sourceObject)
			{
				continue;
			}

			CookieRunObstacleComponent* candidateObstacle = candidate->GetComponent<CookieRunObstacleComponent>();
			if (candidateObstacle == nullptr || !candidateObstacle->RequiresDoubleJump())
			{
				continue;
			}

			if (fabs(candidate->Position().x - sourceX) < 0.1f)
			{
				return candidate;
			}
		}

		return nullptr;
	}
}

void ObjectPoolComponent::Initialize()
{
}

void ObjectPoolComponent::Release()
{
	m_poolObjects.clear();
}

void ObjectPoolComponent::Start()
{
	CollectChildren();

	// Loop Pool은 시작할 때 조각을 모두 켜 둔다.
	if (m_repositionInsteadOfDisable)
	{
		for (std::vector<GameObject*>::iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
		{
			GameObject* object = *itr;
			if (object != nullptr)
			{
				object->SetActive(true);
			}
		}
		return;
	}

	DeactivateAllPoolObjects();
	ActivateInitialPoolObjects();
}

void ObjectPoolComponent::Update()
{
}

void ObjectPoolComponent::CollectChildren()
{
	m_poolObjects.clear();

	GameObject* owner = GetGameObject();
	if (owner == nullptr)
	{
		return;
	}

	std::vector<GameObject*>* children = owner->GetChild();
	if (children == nullptr)
	{
		return;
	}

	for (std::vector<GameObject*>::iterator itr = children->begin(); itr != children->end(); ++itr)
	{
		if (*itr == nullptr)
		{
			continue;
		}

		m_poolObjects.push_back(*itr);
	}

	std::sort(
		m_poolObjects.begin(),
		m_poolObjects.end(),
		[](GameObject* lhs, GameObject* rhs)
		{
			if (lhs == nullptr || rhs == nullptr)
			{
				return lhs < rhs;
			}

			return lhs->Position().x < rhs->Position().x;
		});
}

void ObjectPoolComponent::DeactivateAllPoolObjects()
{
	for (std::vector<GameObject*>::iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* object = *itr;
		if (object == nullptr)
		{
			continue;
		}

		object->SetActive(false);
	}
}

void ObjectPoolComponent::ActivateInitialPoolObjects()
{
	bool activatedAnyObject = false;
	float maxObjectWidth = 0.0f;

	for (std::vector<GameObject*>::const_iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* object = *itr;
		if (object == nullptr)
		{
			continue;
		}

		maxObjectWidth = max(maxObjectWidth, object->Size2D().x);
	}

	float initialRightRange = m_rightSpawnX;
	if (m_repositionInsteadOfDisable)
	{
		initialRightRange += maxObjectWidth * 2.0f;
	}

	for (std::vector<GameObject*>::iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* object = *itr;
		if (object == nullptr)
		{
			continue;
		}

		const float halfWidth = object->Size2D().x * 0.5f;
		const float leftEdge = object->Position().x - halfWidth;
		const float rightEdge = object->Position().x + halfWidth;
		const bool overlapsInitialActiveRange =
			rightEdge >= m_leftBound &&
			leftEdge <= initialRightRange;

		if (overlapsInitialActiveRange)
		{
			object->SetActive(true);
			activatedAnyObject = true;
		}
	}

	if (!activatedAnyObject && !m_poolObjects.empty() && m_poolObjects.front() != nullptr)
	{
		m_poolObjects.front()->SetActive(true);
	}
}

void ObjectPoolComponent::RecycleObject(GameObject* objectToRecycle)
{
	if (objectToRecycle == nullptr)
	{
		return;
	}

	if (!m_repositionInsteadOfDisable)
	{
		objectToRecycle->SetActive(false);

		if (GameObject* partner = FindDoubleJumpPartner(objectToRecycle, m_poolObjects, objectToRecycle->Position().x))
		{
			partner->SetActive(false);
		}

		return;
	}

	float rightMostEdge = m_rightSpawnX;
	for (std::vector<GameObject*>::const_iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* current = *itr;
		if (current == nullptr || current == objectToRecycle)
		{
			continue;
		}

		const float currentHalfWidth = current->Size2D().x * 0.5f;
		rightMostEdge = max(rightMostEdge, current->Position().x + currentHalfWidth);
	}

	const float recycleHalfWidth = objectToRecycle->Size2D().x * 0.5f;
	D3DXVECTOR3 position = objectToRecycle->Position();
	position.x = rightMostEdge + recycleHalfWidth + m_spacing;
	objectToRecycle->SetPosition(position);
	objectToRecycle->SetActive(true);
}

GameObject* ObjectPoolComponent::ActivateNextInactiveObject()
{
	GameObject* objectToActivate = nullptr;
	for (std::vector<GameObject*>::iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* object = *itr;
		if (object != nullptr && !object->GetActive())
		{
			objectToActivate = object;
			break;
		}
	}

	if (objectToActivate == nullptr)
	{
		return nullptr;
	}

	const float originalX = objectToActivate->Position().x;
	float rightMostEdge = m_rightSpawnX;
	for (std::vector<GameObject*>::const_iterator itr = m_poolObjects.begin(); itr != m_poolObjects.end(); ++itr)
	{
		GameObject* current = *itr;
		if (current == nullptr || current == objectToActivate || !current->GetActive())
		{
			continue;
		}

		const float currentHalfWidth = current->Size2D().x * 0.5f;
		rightMostEdge = max(rightMostEdge, current->Position().x + currentHalfWidth);
	}

	const float activateHalfWidth = objectToActivate->Size2D().x * 0.5f;
	D3DXVECTOR3 position = objectToActivate->Position();
	position.x = rightMostEdge + activateHalfWidth + m_spacing;
	objectToActivate->SetPosition(position);
	objectToActivate->SetActive(true);

	if (GameObject* partner = FindDoubleJumpPartner(objectToActivate, m_poolObjects, originalX))
	{
		D3DXVECTOR3 partnerPosition = partner->Position();
		partnerPosition.x = position.x;
		partner->SetPosition(partnerPosition);
		partner->SetActive(true);
	}

	return objectToActivate;
}

const std::vector<GameObject*>& ObjectPoolComponent::GetPoolObjects() const
{
	return m_poolObjects;
}

float ObjectPoolComponent::GetLeftBound() const
{
	return m_leftBound;
}

void ObjectPoolComponent::SetLeftBound(float leftBound)
{
	m_leftBound = leftBound;
}

float ObjectPoolComponent::GetRightSpawnX() const
{
	return m_rightSpawnX;
}

void ObjectPoolComponent::SetRightSpawnX(float rightSpawnX)
{
	m_rightSpawnX = rightSpawnX;
}

float ObjectPoolComponent::GetSpacing() const
{
	return m_spacing;
}

void ObjectPoolComponent::SetSpacing(float spacing)
{
	m_spacing = spacing;
}

bool ObjectPoolComponent::GetRepositionInsteadOfDisable() const
{
	return m_repositionInsteadOfDisable;
}

void ObjectPoolComponent::SetRepositionInsteadOfDisable(bool repositionInsteadOfDisable)
{
	m_repositionInsteadOfDisable = repositionInsteadOfDisable;
}

void ObjectPoolComponent::SetPoolMode(bool repositionInsteadOfDisable)
{
	m_repositionInsteadOfDisable = repositionInsteadOfDisable;
}

const std::string& ObjectPoolComponent::GetPoolTag() const
{
	return m_poolTag;
}

void ObjectPoolComponent::SetPoolTag(const std::string& poolTag)
{
	m_poolTag = poolTag;
}

const char* ObjectPoolComponent::GetInspectorName() const
{
	return "Object Pool";
}

void ObjectPoolComponent::DrawInspector()
{
	DrawSharedInspector(true);
}

void ObjectPoolComponent::DrawSharedInspector(bool showModeToggle)
{
	SyncImGuiContextForCurrentModule();
	ImGui::Text("Pool Count: %d", static_cast<int>(m_poolObjects.size()));
	ImGui::Text("Pool Mode: %s", m_repositionInsteadOfDisable ? "Loop Pool" : "Spawn Pool");
	ImGui::TextDisabled(
		m_repositionInsteadOfDisable
		? "Loop Pool: keeps segments active and repositions them."
		: "Spawn Pool: starts inactive and only activates needed objects.");
	ImGui::DragFloat("Left Bound", &m_leftBound, 1.0f);
	ImGui::DragFloat("Right Spawn X", &m_rightSpawnX, 1.0f);
	ImGui::DragFloat("Spacing", &m_spacing, 1.0f, 0.0f, 5000.0f);
	if (showModeToggle)
	{
		ImGui::Checkbox("Use Loop Pool Mode", &m_repositionInsteadOfDisable);
	}

	char poolTagBuffer[128] = {};
	strcpy_s(poolTagBuffer, m_poolTag.c_str());
	if (ImGui::InputText("Pool Tag", poolTagBuffer, IM_ARRAYSIZE(poolTagBuffer)))
	{
		m_poolTag = poolTagBuffer;
	}

	if (ImGui::Button("Collect Children"))
	{
		CollectChildren();
	}
}

const char* ObjectPoolComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string ObjectPoolComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"leftBound\": " << m_leftBound << ", ";
	oss << "\"rightSpawnX\": " << m_rightSpawnX << ", ";
	oss << "\"spacing\": " << m_spacing << ", ";
	oss << "\"repositionInsteadOfDisable\": " << (m_repositionInsteadOfDisable ? "true" : "false") << ", ";
	oss << "\"poolTag\": \"" << SceneJson::EscapeString(m_poolTag) << "\"";
	oss << " }";
	return oss.str();
}

bool ObjectPoolComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadFloat(componentJson, "leftBound", m_leftBound);
	SceneJson::ReadFloat(componentJson, "rightSpawnX", m_rightSpawnX);
	SceneJson::ReadFloat(componentJson, "spacing", m_spacing);
	SceneJson::ReadBool(componentJson, "repositionInsteadOfDisable", m_repositionInsteadOfDisable);
	SceneJson::ReadString(componentJson, "poolTag", m_poolTag);
	return true;
}
