#include "pch.h"
#include "SampleSpinComponent.h"

#include "GameObject.h"
#include "SceneJsonUtility.h"

void SampleSpinComponent::Initialize()
{
}

void SampleSpinComponent::Release()
{
}

void SampleSpinComponent::Start()
{
}

void SampleSpinComponent::Update()
{
	if (!m_enabled || m_gameObj == nullptr)
	{
		return;
	}

	if (m_useYAxis)
	{
		m_gameObj->SetAngleY(m_gameObj->GetAngleY() + m_spinSpeed);
	}

	if (m_useZAxis)
	{
		m_gameObj->SetAngleZ(m_gameObj->GetAngleZ() + m_spinSpeed);
	}
}

const char* SampleSpinComponent::GetInspectorName() const
{
	return "SampleSpinComponent";
}

void SampleSpinComponent::DrawInspector()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::InputFloat("Spin Speed", &m_spinSpeed);
	ImGui::Checkbox("Use Y Axis", &m_useYAxis);
	ImGui::Checkbox("Use Z Axis", &m_useZAxis);
}

const char* SampleSpinComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string SampleSpinComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"spinSpeed\": " << m_spinSpeed << ", ";
	oss << "\"useYAxis\": " << (m_useYAxis ? "true" : "false") << ", ";
	oss << "\"useZAxis\": " << (m_useZAxis ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool SampleSpinComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "enabled", m_enabled);
	SceneJson::ReadFloat(componentJson, "spinSpeed", m_spinSpeed);
	SceneJson::ReadBool(componentJson, "useYAxis", m_useYAxis);
	SceneJson::ReadBool(componentJson, "useZAxis", m_useZAxis);
	return true;
}
