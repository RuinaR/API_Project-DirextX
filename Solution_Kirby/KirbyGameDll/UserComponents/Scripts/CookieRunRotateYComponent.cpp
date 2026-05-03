#include "pch.h"
#include "CookieRunRotateYComponent.h"

#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "MainFrame.h"
#include "SceneJsonUtility.h"

void CookieRunRotateYComponent::Initialize()
{
}

void CookieRunRotateYComponent::Release()
{
}

void CookieRunRotateYComponent::Start()
{
}

void CookieRunRotateYComponent::Update()
{
	GameObject* owner = GetGameObject();
	MainFrame* mainFrame = MainFrame::GetInstance();
	if (owner == nullptr || mainFrame == nullptr)
	{
		return;
	}

	const float deltaTime = static_cast<float>(mainFrame->DeltaTime());
	if (deltaTime <= 0.0f)
	{
		return;
	}

	float angleY = owner->GetAngleY() + (m_rotationSpeed * deltaTime);
	const float fullRotation = D3DX_PI * 2.0f;
	while (angleY > fullRotation)
	{
		angleY -= fullRotation;
	}

	while (angleY < -fullRotation)
	{
		angleY += fullRotation;
	}

	owner->SetAngleY(angleY);
}

const char* CookieRunRotateYComponent::GetInspectorName() const
{
	return "CookieRun Rotate Y";
}

void CookieRunRotateYComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::DragFloat("Rotation Speed", &m_rotationSpeed, 0.01f, -10.0f, 10.0f);
}

const char* CookieRunRotateYComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunRotateYComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"rotationSpeed\": " << m_rotationSpeed;
	oss << " }";
	return oss.str();
}

bool CookieRunRotateYComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadFloat(componentJson, "rotationSpeed", m_rotationSpeed);
	return true;
}
