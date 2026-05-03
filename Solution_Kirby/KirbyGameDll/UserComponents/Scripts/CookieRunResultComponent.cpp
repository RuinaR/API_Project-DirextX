#include "pch.h"
#include "CookieRunResultComponent.h"

#include "CookieRunGameManagerComponent.h"
#include "DebugLog.h"
#include "ImGuiContextBridge.h"
#include "ObjectManager.h"
#include "SceneJsonUtility.h"
#include "UILabel.h"

void CookieRunResultComponent::Initialize()
{
}

void CookieRunResultComponent::Release()
{
}

void CookieRunResultComponent::Start()
{
	ResolveFallbackReferences();
	UpdateResultScoreUI();
	if (!m_loggedResultEntry)
	{
		DebugLog::Log("ResultScene entered.");
		m_loggedResultEntry = true;
	}
}

void CookieRunResultComponent::Update()
{
	UpdateResultScoreUI();
}

const char* CookieRunResultComponent::GetInspectorName() const
{
	return "CookieRun Result";
}

void CookieRunResultComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::Text("Last Score: %d", CookieRunGameManagerComponent::GetLastScore());
}

const char* CookieRunResultComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunResultComponent::Serialize() const
{
	return "{}";
}

bool CookieRunResultComponent::Deserialize(const std::string& componentJson)
{
	(void)componentJson;
	return true;
}

void CookieRunResultComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterComponentRef("ResultScoreLabel Component", &m_resultScoreLabelComponent, "UILabel");
}

void CookieRunResultComponent::ResolveFallbackReferences()
{
	if (m_resultScoreLabelComponent == nullptr)
	{
		if (GameObject* object = ObjectManager::GetInstance()->FindObjectByName("ResultScoreLabel"))
		{
			m_resultScoreLabelComponent = object->GetComponent<UILabel>();
		}
	}
}

void CookieRunResultComponent::UpdateResultScoreUI()
{
	if (UILabel* label = GetResultScoreLabel())
	{
		label->SetText(ConvertToWideString(
			"Final Score: " + std::to_string(CookieRunGameManagerComponent::GetLastScore())));
	}
}

UILabel* CookieRunResultComponent::GetResultScoreLabel() const
{
	return dynamic_cast<UILabel*>(m_resultScoreLabelComponent);
}
