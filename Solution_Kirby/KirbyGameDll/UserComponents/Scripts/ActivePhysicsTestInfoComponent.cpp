#include "pch.h"
#include "ActivePhysicsTestInfoComponent.h"

#include "../../UserActions/Scripts/ActivePhysicsTestActions.h"
#include "ActivePhysicsTestStatusComponent.h"
#include "Collider2D.h"
#include "Editor/EditorInspectorWindow.h"
#include "GameObject.h"
#include "SceneJsonUtility.h"
#include "UILabel.h"

namespace
{
	Collider2D* FindFirstCollider(GameObject* gameObject)
	{
		if (gameObject == nullptr || gameObject->GetComponentVec() == nullptr)
		{
			return nullptr;
		}

		vector<Component*>* components = gameObject->GetComponentVec();
		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
			if (collider != nullptr)
			{
				return collider;
			}
		}

		return nullptr;
	}
}

void ActivePhysicsTestInfoComponent::Initialize()
{
}

void ActivePhysicsTestInfoComponent::Release()
{
	SetActivePhysicsTestPlayerObject(nullptr);
	m_labelObject = nullptr;
	m_playerObject = nullptr;
	m_labelObjectId = -1;
	m_playerObjectId = -1;
}

void ActivePhysicsTestInfoComponent::Start()
{
	SetActivePhysicsTestPlayerObject(m_playerObject);
	RefreshLabel();
}

void ActivePhysicsTestInfoComponent::Update()
{
	RefreshLabel();
}

const char* ActivePhysicsTestInfoComponent::GetInspectorName() const
{
	return "Active Physics Test Info";
}

void ActivePhysicsTestInfoComponent::DrawInspector()
{
	EditorInspectorWindow::DrawGameObjectReferenceField("Info Label", m_labelObject, m_labelObjectId);
	EditorInspectorWindow::DrawGameObjectReferenceField("Player Object", m_playerObject, m_playerObjectId);
}

const char* ActivePhysicsTestInfoComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string ActivePhysicsTestInfoComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	if (m_labelObjectId >= 0)
	{
		oss << "\"labelObjectId\": " << m_labelObjectId;
		if (m_playerObjectId >= 0)
		{
			oss << ", ";
		}
	}
	if (m_playerObjectId >= 0)
	{
		oss << "\"playerObjectId\": " << m_playerObjectId;
	}
	oss << " }";
	return oss.str();
}

bool ActivePhysicsTestInfoComponent::Deserialize(const std::string& componentJson)
{
	m_labelObject = nullptr;
	m_playerObject = nullptr;
	m_labelObjectId = -1;
	m_playerObjectId = -1;
	SceneJson::ReadInt(componentJson, "labelObjectId", m_labelObjectId);
	SceneJson::ReadInt(componentJson, "playerObjectId", m_playerObjectId);
	return true;
}

void ActivePhysicsTestInfoComponent::ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap)
{
	m_labelObject = nullptr;
	m_playerObject = nullptr;

	std::unordered_map<int, GameObject*>::const_iterator labelItr = objectMap.find(m_labelObjectId);
	if (labelItr != objectMap.end())
	{
		m_labelObject = labelItr->second;
	}

	std::unordered_map<int, GameObject*>::const_iterator playerItr = objectMap.find(m_playerObjectId);
	if (playerItr != objectMap.end())
	{
		m_playerObject = playerItr->second;
	}

	SetActivePhysicsTestPlayerObject(m_playerObject);
}

void ActivePhysicsTestInfoComponent::RefreshLabel()
{
	if (m_labelObject == nullptr)
	{
		return;
	}

	UILabel* label = m_labelObject->GetComponent<UILabel>();
	if (label == nullptr)
	{
		return;
	}

	if (m_playerObject == nullptr)
	{
		label->SetText(L"Player reference is missing.");
		return;
	}

	const bool isActive = m_playerObject->GetActive();
	Collider2D* collider = FindFirstCollider(m_playerObject);
	const bool isTrigger = collider != nullptr && collider->GetTrigger();
	ActivePhysicsTestStatusComponent* status = m_playerObject->GetComponent<ActivePhysicsTestStatusComponent>();

	std::ostringstream oss;
	oss << "Player Active: " << (isActive ? "true" : "false") << "\n";
	oss << "Player Trigger: " << (isTrigger ? "true" : "false") << "\n";

	if (status != nullptr)
	{
		oss << "Collision Contacts: " << status->GetCollisionCount() << "\n";
		oss << "Trigger Contacts: " << status->GetTriggerCount() << "\n";
		oss << "Enable Count: " << status->GetEnableCount() << "\n";
		oss << "Disable Count: " << status->GetDisableCount() << "\n";
		oss << "Collision Enter Count: " << status->GetCollisionEnterCount() << "\n";
		oss << "Trigger Enter Count: " << status->GetTriggerEnterCount() << "\n";
		oss << "Last Event: " << status->GetLastEventText() << "\n";
		oss << status->GetRecentEventsText();
	}
	else
	{
		oss << "Status Component: missing";
	}

	label->SetText(ConvertToWideString(oss.str()));
}
