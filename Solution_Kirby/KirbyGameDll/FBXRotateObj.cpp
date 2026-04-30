#include "pch.h"
#include "FBXRotateObj.h"
#include "SceneJsonUtility.h"

void FBXRotateObj::Initialize()
{
}

void FBXRotateObj::Release()
{
}

void FBXRotateObj::Start()
{
}

void FBXRotateObj::Update()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	if (m_rotateX)
	{
		m_gameObj->SetAngleX(m_gameObj->GetAngleX() + m_rotateSpeed);
	}
	if (m_rotateY)
	{
		m_gameObj->SetAngleY(m_gameObj->GetAngleY() + m_rotateSpeed);
	}
	if (m_rotateZ)
	{
		m_gameObj->SetAngleZ(m_gameObj->GetAngleZ() + m_rotateSpeed);
	}
}

void FBXRotateObj::DrawInspector()
{
	ImGui::InputFloat("Rotate Speed", &m_rotateSpeed);
	ImGui::Checkbox("Rotate X", &m_rotateX);
	ImGui::Checkbox("Rotate Y", &m_rotateY);
	ImGui::Checkbox("Rotate Z", &m_rotateZ);
}

const char* FBXRotateObj::GetSerializableType() const
{
	return kComponentType;
}

std::string FBXRotateObj::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"rotateSpeed\": " << m_rotateSpeed << ", ";
	oss << "\"rotateX\": " << (m_rotateX ? "true" : "false") << ", ";
	oss << "\"rotateY\": " << (m_rotateY ? "true" : "false") << ", ";
	oss << "\"rotateZ\": " << (m_rotateZ ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool FBXRotateObj::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadFloat(componentJson, "rotateSpeed", m_rotateSpeed);
	SceneJson::ReadBool(componentJson, "rotateX", m_rotateX);
	SceneJson::ReadBool(componentJson, "rotateY", m_rotateY);
	SceneJson::ReadBool(componentJson, "rotateZ", m_rotateZ);
	return true;
}
