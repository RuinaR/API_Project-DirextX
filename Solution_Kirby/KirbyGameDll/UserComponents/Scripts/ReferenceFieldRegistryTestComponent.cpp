#include "pch.h"
#include "ReferenceFieldRegistryTestComponent.h"

#include "Editor/EditorInspectorWindow.h"
#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "SceneJsonUtility.h"

void ReferenceFieldRegistryTestComponent::Initialize()
{
}

void ReferenceFieldRegistryTestComponent::Release()
{
	m_manualTargetObject = nullptr;
	m_manualTargetObjectId = -1;
	m_registryTargetObject = nullptr;
	m_anyComponentRef = nullptr;
	m_rigidbodyComponentRef = nullptr;
}

void ReferenceFieldRegistryTestComponent::Start()
{
}

void ReferenceFieldRegistryTestComponent::Update()
{
}

const char* ReferenceFieldRegistryTestComponent::GetInspectorName() const
{
	return "Reference Field Registry Test";
}

void ReferenceFieldRegistryTestComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();

	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::Separator();
	ImGui::Text("Manual Reference Field");
	EditorInspectorWindow::DrawGameObjectReferenceField("Manual Target Object", m_manualTargetObject, m_manualTargetObjectId);

	ImGui::Separator();
	ImGui::Text("Registry Preview");
	ImGui::TextWrapped("The fields below are registered through RegisterReferenceFields().");
	ImGui::Text("Manual GameObject: %s", DescribeGameObject(m_manualTargetObject).c_str());
	ImGui::Text("Registry GameObject: %s", DescribeGameObject(m_registryTargetObject).c_str());
	ImGui::Text("Any Component: %s", DescribeComponent(m_anyComponentRef).c_str());
	ImGui::Text("Rigidbody2D Only: %s", DescribeComponent(m_rigidbodyComponentRef).c_str());
}

const char* ReferenceFieldRegistryTestComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string ReferenceFieldRegistryTestComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false");
	if (m_manualTargetObjectId >= 0)
	{
		oss << ", \"manualTargetObjectId\": " << m_manualTargetObjectId;
	}
	oss << " }";
	return oss.str();
}

bool ReferenceFieldRegistryTestComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "enabled", m_enabled);
	m_manualTargetObject = nullptr;
	m_manualTargetObjectId = -1;
	m_registryTargetObject = nullptr;
	m_anyComponentRef = nullptr;
	m_rigidbodyComponentRef = nullptr;
	SceneJson::ReadInt(componentJson, "manualTargetObjectId", m_manualTargetObjectId);
	return true;
}

void ReferenceFieldRegistryTestComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterGameObjectRef("Registry Target Object", &m_registryTargetObject);
	registry.RegisterComponentRef("Any Component", &m_anyComponentRef);
	registry.RegisterComponentRef("Rigidbody2D Only", &m_rigidbodyComponentRef, "Rigidbody2D");
}

void ReferenceFieldRegistryTestComponent::ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap)
{
	m_manualTargetObject = nullptr;
	if (m_manualTargetObjectId >= 0)
	{
		std::unordered_map<int, GameObject*>::const_iterator itr = objectMap.find(m_manualTargetObjectId);
		if (itr != objectMap.end())
		{
			m_manualTargetObject = itr->second;
		}
	}

	m_registryTargetObject = nullptr;
	m_anyComponentRef = nullptr;
	m_rigidbodyComponentRef = nullptr;
}

std::string ReferenceFieldRegistryTestComponent::DescribeGameObject(GameObject* gameObject) const
{
	if (gameObject == nullptr)
	{
		return "(None)";
	}

	return gameObject->GetName() + " [id=" + std::to_string(gameObject->GetId()) + "]";
}

std::string ReferenceFieldRegistryTestComponent::DescribeComponent(Component* component) const
{
	if (component == nullptr)
	{
		return "(None)";
	}

	GameObject* owner = component->GetGameObject();
	std::string text = component->GetInspectorName();
	text += " @ ";
	text += owner != nullptr ? owner->GetName() : "(Detached)";
	return text;
}
