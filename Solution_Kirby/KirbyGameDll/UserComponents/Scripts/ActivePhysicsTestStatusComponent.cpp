#include "pch.h"
#include "ActivePhysicsTestStatusComponent.h"

void ActivePhysicsTestStatusComponent::Initialize()
{
}

void ActivePhysicsTestStatusComponent::Release()
{
	m_activeCollisionCount = 0;
	m_activeTriggerCount = 0;
	m_enableCount = 0;
	m_disableCount = 0;
	m_collisionEnterCount = 0;
	m_triggerEnterCount = 0;
	m_lastEvent = "Released";
	m_recentEvents.clear();
}

void ActivePhysicsTestStatusComponent::Start()
{
}

void ActivePhysicsTestStatusComponent::Update()
{
}

const char* ActivePhysicsTestStatusComponent::GetInspectorName() const
{
	return "Active Physics Test Status";
}

void ActivePhysicsTestStatusComponent::DrawInspector()
{
	ImGui::Text("Active Collisions: %d", m_activeCollisionCount);
	ImGui::Text("Active Triggers: %d", m_activeTriggerCount);
	ImGui::Text("Enable Count: %d", m_enableCount);
	ImGui::Text("Disable Count: %d", m_disableCount);
	ImGui::Text("Collision Enter Count: %d", m_collisionEnterCount);
	ImGui::Text("Trigger Enter Count: %d", m_triggerEnterCount);
	ImGui::Text("Last Event: %s", m_lastEvent.c_str());
	ImGui::Separator();
	for (size_t i = 0; i < m_recentEvents.size(); ++i)
	{
		ImGui::Text("%zu. %s", i + 1, m_recentEvents[i].c_str());
	}
}

const char* ActivePhysicsTestStatusComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string ActivePhysicsTestStatusComponent::Serialize() const
{
	return "{}";
}

bool ActivePhysicsTestStatusComponent::Deserialize(const std::string& componentJson)
{
	(void)componentJson;
	return true;
}

int ActivePhysicsTestStatusComponent::GetCollisionCount() const
{
	return m_activeCollisionCount;
}

int ActivePhysicsTestStatusComponent::GetTriggerCount() const
{
	return m_activeTriggerCount;
}

int ActivePhysicsTestStatusComponent::GetEnableCount() const
{
	return m_enableCount;
}

int ActivePhysicsTestStatusComponent::GetDisableCount() const
{
	return m_disableCount;
}

int ActivePhysicsTestStatusComponent::GetCollisionEnterCount() const
{
	return m_collisionEnterCount;
}

int ActivePhysicsTestStatusComponent::GetTriggerEnterCount() const
{
	return m_triggerEnterCount;
}

const char* ActivePhysicsTestStatusComponent::GetLastEventText() const
{
	return m_lastEvent.c_str();
}

std::string ActivePhysicsTestStatusComponent::GetRecentEventsText() const
{
	if (m_recentEvents.empty())
	{
		return "Recent Events:\n- None";
	}

	std::ostringstream oss;
	oss << "Recent Events:";
	for (size_t i = 0; i < m_recentEvents.size(); ++i)
	{
		oss << "\n- " << m_recentEvents[i];
	}
	return oss.str();
}

void ActivePhysicsTestStatusComponent::CollisionEnter(Collider2D* other)
{
	(void)other;
	++m_activeCollisionCount;
	++m_collisionEnterCount;
	PushEvent("CollisionEnter");
}

void ActivePhysicsTestStatusComponent::CollisionExit(Collider2D* other)
{
	(void)other;
	if (m_activeCollisionCount > 0)
	{
		--m_activeCollisionCount;
	}
	PushEvent("CollisionExit");
}

void ActivePhysicsTestStatusComponent::TriggerEnter(Collider2D* other)
{
	(void)other;
	++m_activeTriggerCount;
	++m_triggerEnterCount;
	PushEvent("TriggerEnter");
}

void ActivePhysicsTestStatusComponent::TriggerExit(Collider2D* other)
{
	(void)other;
	if (m_activeTriggerCount > 0)
	{
		--m_activeTriggerCount;
	}
	PushEvent("TriggerExit");
}

void ActivePhysicsTestStatusComponent::Enable()
{
	++m_enableCount;
	PushEvent("OnEnable");
}

void ActivePhysicsTestStatusComponent::Disable()
{
	++m_disableCount;
	PushEvent("OnDisable");
}

void ActivePhysicsTestStatusComponent::LBtnDown()
{
	PushEvent("OnLBtnDown");
}

void ActivePhysicsTestStatusComponent::LBtnUp()
{
	PushEvent("OnLBtnUp");
}

void ActivePhysicsTestStatusComponent::RBtnDown()
{
	PushEvent("OnRBtnDown");
}

void ActivePhysicsTestStatusComponent::RBtnUp()
{
	PushEvent("OnRBtnUp");
}

void ActivePhysicsTestStatusComponent::MouseHoverEnter()
{
	PushEvent("OnMouseHoverEnter");
}

void ActivePhysicsTestStatusComponent::MouseHoverExit()
{
	PushEvent("OnMouseHoverExit");
}

void ActivePhysicsTestStatusComponent::PushEvent(const char* text)
{
	m_lastEvent = text != nullptr ? text : "";

	if (m_recentEvents.size() >= 5)
	{
		m_recentEvents.erase(m_recentEvents.begin());
	}

	m_recentEvents.push_back(m_lastEvent);
}
