#include "pch.h"
#include "ObjectManagerSearchApiTestComponent.h"

#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "ObjectManager.h"
#include "Editor/EditorSceneWorkflow.h"
#include "SceneDataManager.h"
#include "SceneJsonUtility.h"
#include "WindowFrame.h"

namespace
{
	constexpr size_t kMaxRecentLogs = 10;
}

void ObjectManagerSearchApiTestComponent::Initialize()
{
}

void ObjectManagerSearchApiTestComponent::Release()
{
	m_recentLogs.clear();
}

void ObjectManagerSearchApiTestComponent::Start()
{
}

void ObjectManagerSearchApiTestComponent::Update()
{
}

const char* ObjectManagerSearchApiTestComponent::GetInspectorName() const
{
	return "ObjectManager Search API Test";
}

void ObjectManagerSearchApiTestComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::Checkbox("Enabled", &m_enabled);

	char tagBuffer[128] = {};
	strcpy_s(tagBuffer, m_testTag.c_str());
	if (ImGui::InputText("Test Tag", tagBuffer, IM_ARRAYSIZE(tagBuffer)))
	{
		m_testTag = tagBuffer;
	}

	ImGui::DragInt("Spawn Count", &m_spawnCount, 1.0f, 1, 20);
	ImGui::DragFloat("Spacing X", &m_spacingX, 1.0f, 10.0f, 1000.0f, "%.1f");

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "ObjectManager unavailable.");
		return;
	}

	if (!m_enabled)
	{
		ImGui::TextDisabled("Component disabled.");
	}

	if (ImGui::Button("Setup Test Objects"))
	{
		SetupTestObjects();
	}

	ImGui::SameLine();
	if (ImGui::Button("Flush Pending"))
	{
		objectManager->FlushPendingObjects();
		PushLog("FlushPendingObjects executed.");
		RunSearchSnapshot("After Flush");
		MarkCurrentSceneDirty();
	}

	if (ImGui::Button("Run Search Snapshot"))
	{
		RunSearchSnapshot("Current Snapshot");
	}

	ImGui::SameLine();
	if (ImGui::Button("Run Legacy Snapshot"))
	{
		RunLegacySearchSnapshot("Legacy Snapshot");
	}

	if (ImGui::Button("Destroy First By Tag"))
	{
		RunDestroyFirstByTag();
	}

	ImGui::SameLine();
	if (ImGui::Button("Destroy All By Tag"))
	{
		RunDestroyAllByTag();
	}

	ImGui::SameLine();
	if (ImGui::Button("Destroy By Name"))
	{
		RunDestroyByName();
	}

	const std::vector<GameObject*> matchedObjects = objectManager->FindObjectsByTag(m_testTag);
	ImGui::Separator();
	ImGui::Text("Current Matches By Tag: %d", static_cast<int>(matchedObjects.size()));
	ImGui::Text("First By Tag: %s", DescribeObject(objectManager->FindFirstObjectByTag(m_testTag)).c_str());
	ImGui::Text("By Name (temporary tag-based): %s", DescribeObject(objectManager->FindObjectByName(m_testTag)).c_str());

	ImGui::Separator();
	ImGui::Text("Recent Logs");
	for (size_t i = 0; i < m_recentLogs.size(); ++i)
	{
		ImGui::Text("%zu. %s", i + 1, m_recentLogs[i].c_str());
	}
}

const char* ObjectManagerSearchApiTestComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string ObjectManagerSearchApiTestComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"testTag\": \"" << SceneJson::EscapeString(m_testTag) << "\", ";
	oss << "\"spawnCount\": " << m_spawnCount << ", ";
	oss << "\"spacingX\": " << m_spacingX;
	oss << " }";
	return oss.str();
}

bool ObjectManagerSearchApiTestComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "enabled", m_enabled);
	SceneJson::ReadString(componentJson, "testTag", m_testTag);
	SceneJson::ReadInt(componentJson, "spawnCount", m_spawnCount);
	SceneJson::ReadFloat(componentJson, "spacingX", m_spacingX);
	return true;
}

void ObjectManagerSearchApiTestComponent::SetupTestObjects()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!m_enabled || objectManager == nullptr)
	{
		PushLog("Setup skipped: component disabled or ObjectManager unavailable.");
		return;
	}

	const D3DXVECTOR3 basePosition = m_gameObj != nullptr ? m_gameObj->Position() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < m_spawnCount; ++i)
	{
		GameObject* obj = new GameObject();
		obj->SetTag(m_testTag);
		obj->SetPosition(D3DXVECTOR3(basePosition.x + (m_spacingX * i), basePosition.y, basePosition.z));
		obj->InitializeSet();
	}

	PushLog("Setup created duplicate-tag objects in pendingAddObjects.");
	RunSearchSnapshot("After Setup Pending");
	RunLegacySearchSnapshot("After Setup Pending");
	MarkCurrentSceneDirty();
}

void ObjectManagerSearchApiTestComponent::RunSearchSnapshot(const char* label)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		PushLog("Search snapshot failed: ObjectManager unavailable.");
		return;
	}

	GameObject* firstByTag = objectManager->FindFirstObjectByTag(m_testTag);
	GameObject* byName = objectManager->FindObjectByName(m_testTag);
	const std::vector<GameObject*> matchedObjects = objectManager->FindObjectsByTag(m_testTag);

	std::ostringstream oss;
	oss << (label != nullptr ? label : "Search Snapshot")
		<< " | firstByTag=" << DescribeObject(firstByTag)
		<< " | byName=" << DescribeObject(byName)
		<< " | count=" << matchedObjects.size();
	PushLog(oss.str());
}

void ObjectManagerSearchApiTestComponent::RunLegacySearchSnapshot(const char* label)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		PushLog("Legacy search snapshot failed: ObjectManager unavailable.");
		return;
	}

	GameObject* legacyFind = objectManager->FindObject(m_testTag);
	std::ostringstream oss;
	oss << (label != nullptr ? label : "Legacy Snapshot")
		<< " | FindObject(tag)=" << DescribeObject(legacyFind);
	PushLog(oss.str());
}

void ObjectManagerSearchApiTestComponent::RunDestroyFirstByTag()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		PushLog("DestroyFirstByTag failed: ObjectManager unavailable.");
		return;
	}

	const bool destroyed = objectManager->DestroyFirstObjectByTag(m_testTag);
	std::ostringstream oss;
	oss << "DestroyFirstObjectByTag(" << m_testTag << ") => " << (destroyed ? "true" : "false")
		<< ", remainingVisibleMatches=" << objectManager->FindObjectsByTag(m_testTag).size();
	PushLog(oss.str());
	MarkCurrentSceneDirty();
}

void ObjectManagerSearchApiTestComponent::RunDestroyAllByTag()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		PushLog("DestroyObjectsByTag failed: ObjectManager unavailable.");
		return;
	}

	const int destroyRequestedCount = objectManager->DestroyObjectsByTag(m_testTag);
	std::ostringstream oss;
	oss << "DestroyObjectsByTag(" << m_testTag << ") => requested " << destroyRequestedCount
		<< ", remainingVisibleMatches=" << objectManager->FindObjectsByTag(m_testTag).size();
	PushLog(oss.str());
	MarkCurrentSceneDirty();
}

void ObjectManagerSearchApiTestComponent::RunDestroyByName()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		PushLog("DestroyObjectByName failed: ObjectManager unavailable.");
		return;
	}

	const bool destroyed = objectManager->DestroyObjectByName(m_testTag);
	std::ostringstream oss;
	oss << "DestroyObjectByName(" << m_testTag << ") => " << (destroyed ? "true" : "false")
		<< " (temporary tag-based implementation)";
	PushLog(oss.str());
	MarkCurrentSceneDirty();
}

void ObjectManagerSearchApiTestComponent::PushLog(const std::string& text)
{
	if (m_recentLogs.size() >= kMaxRecentLogs)
	{
		m_recentLogs.erase(m_recentLogs.begin());
	}

	m_recentLogs.push_back(text);
	std::cout << "[ObjectManagerSearchApiTest] " << text << std::endl;
}

void ObjectManagerSearchApiTestComponent::MarkCurrentSceneDirty()
{
	if (WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return;
	}

	const char* sceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
	if (sceneName == nullptr || strlen(sceneName) == 0)
	{
		return;
	}

	EditorSceneWorkflow::MarkSceneDirty(sceneName);
}

std::string ObjectManagerSearchApiTestComponent::DescribeObject(GameObject* obj) const
{
	if (obj == nullptr)
	{
		return "(None)";
	}

	std::ostringstream oss;
	oss << obj->GetTag() << " [id=" << obj->GetId() << "]";
	return oss.str();
}
