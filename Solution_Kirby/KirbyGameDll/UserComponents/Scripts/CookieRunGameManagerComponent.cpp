#include "pch.h"
#include "CookieRunGameManagerComponent.h"

#include "DebugLog.h"
#include "GameObject.h"
#include "ImGuiContextBridge.h"
#include "ImageRender.h"
#include "LoopObjectPoolComponent.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "ObjectPoolComponent.h"
#include "Rendering/Camera/Camera.h"
#include "../../UserActions/Scripts/SceneChangeAction.h"
#include "SceneJsonUtility.h"
#include "SpawnObjectPoolComponent.h"
#include "UILabel.h"

int CookieRunGameManagerComponent::s_lastScore = 0;

namespace
{
	const D3DCOLOR kDefaultPlayerColor = 0xffffffff;
	const D3DCOLOR kHitFlashPlayerColor = D3DCOLOR_ARGB(255, 255, 120, 120);
	const D3DCOLOR kScoreCollectPlayerColor = D3DCOLOR_ARGB(255, 180, 255, 140);
	const D3DCOLOR kStartInvinciblePlayerColor = D3DCOLOR_ARGB(255, 150, 210, 255);

	std::wstring ToWideLabel(const std::string& prefix, int value)
	{
		return ConvertToWideString(prefix + std::to_string(value));
	}

	std::wstring ToWideFloatLabel(const std::string& prefix, float value)
	{
		std::ostringstream oss;
		oss.setf(std::ios::fixed);
		oss.precision(2);
		oss << prefix << value;
		return ConvertToWideString(oss.str());
	}

	std::wstring ToWideHpLabel(int currentHp, int maxHp)
	{
		return ConvertToWideString(
			"HP: " + std::to_string(currentHp) + " / " + std::to_string(maxHp));
	}
}

void CookieRunGameManagerComponent::Initialize()
{
}

void CookieRunGameManagerComponent::Release()
{
}

void CookieRunGameManagerComponent::Start()
{
	ResolveFallbackReferences();

	if (MainFrame* mainFrame = MainFrame::GetInstance())
	{
		mainFrame->SetTimeScale(1.0f);
	}

	m_score = 0;
	m_currentHp = m_maxHp;
	m_elapsedTime = 0.0f;
	m_speedIncreaseTimer = 0.0f;
	m_timeScaleIncreaseTimer = 0.0f;
	m_damageInvincibleTimer = 0.0f;
	m_startInvincibleTimer = m_startInvincibleDuration;
	m_hitFlashTimer = 0.0f;
	m_cameraShakeTimer = 0.0f;
	m_currentDifficultyTimeScale = 1.0f;
	m_scoreCollectEffectTimer = 0.0f;
	m_scoreCollectEffectSavedTimeScale = 1.0f;
	m_scoreCollectEffectStartAngleY = 0.0f;
	m_gameOver = false;
	m_scoreCollectEffectActive = false;
	s_lastScore = 0;

	if (Camera* camera = Camera::GetInstance())
	{
		m_baseCameraRotation = camera->GetRotation();
		camera->SetRotation(&m_baseCameraRotation);
	}

	if (UILabel* scoreLabel = GetScoreLabel())
	{
		m_scoreLabelBaseScale = scoreLabel->GetScale();
	}

	if (ImageRender* playerImageRender = GetPlayerImageRender())
	{
		playerImageRender->SetColor(kDefaultPlayerColor);
	}

	UpdateScoreUI();
	UpdateSpeedUI();
	UpdateHpUI();
	DebugLog::Log("GameScene started.");
}

void CookieRunGameManagerComponent::Update()
{
	if (m_gameOver)
	{
		return;
	}

	SanitizeCachedReferences();

	MainFrame* mainFrame = MainFrame::GetInstance();
	if (mainFrame == nullptr)
	{
		return;
	}

	UpdateScoreUI();
	UpdateSpeedUI();
	UpdateHpUI();

	const float deltaTime = static_cast<float>(mainFrame->DeltaTime());
	const float unscaledDeltaTime = static_cast<float>(mainFrame->UnscaledDeltaTime());

	m_elapsedTime += deltaTime;
	m_speedIncreaseTimer += deltaTime;

	if (m_damageInvincibleTimer > 0.0f)
	{
		m_damageInvincibleTimer -= deltaTime;
		if (m_damageInvincibleTimer < 0.0f)
		{
			m_damageInvincibleTimer = 0.0f;
		}
	}

	if (m_startInvincibleTimer > 0.0f)
	{
		m_startInvincibleTimer -= deltaTime;
		if (m_startInvincibleTimer < 0.0f)
		{
			m_startInvincibleTimer = 0.0f;
		}
	}

	UpdateDifficultyTimeScale(deltaTime, unscaledDeltaTime);
	UpdateScoreCollectEffect(unscaledDeltaTime);
	UpdateHitFlashAndCameraShake(deltaTime);

	if (m_speedIncreaseTimer >= m_speedIncreaseInterval)
	{
		m_speedIncreaseTimer = 0.0f;
		m_scrollSpeed += m_speedIncreaseAmount;
		UpdateSpeedUI();
		DebugLog::Log("Scroll speed increased to " + std::to_string(static_cast<int>(m_scrollSpeed)) + ".");
	}
}

void CookieRunGameManagerComponent::AddScore(int amount)
{
	if (m_gameOver || amount <= 0)
	{
		return;
	}

	m_score += amount;
	s_lastScore = m_score;
	UpdateScoreUI();
	DebugLog::Log("Item collected. Score: " + std::to_string(m_score));

	MainFrame* mainFrame = MainFrame::GetInstance();
	if (mainFrame == nullptr)
	{
		return;
	}

	m_scoreCollectEffectSavedTimeScale = m_currentDifficultyTimeScale;
	m_scoreCollectEffectTimer = 0.0f;
	m_scoreCollectEffectActive = true;

	if (m_player != nullptr)
	{
		m_scoreCollectEffectStartAngleY = m_player->GetAngleY();
	}

	mainFrame->SetTimeScale(0.0f);
}

bool CookieRunGameManagerComponent::DamagePlayer(int amount)
{
	if (m_gameOver || amount <= 0 || m_damageInvincibleTimer > 0.0f || m_startInvincibleTimer > 0.0f)
	{
		return false;
	}

	m_currentHp -= amount;
	if (m_currentHp < 0)
	{
		m_currentHp = 0;
	}

	m_damageInvincibleTimer = m_damageInvincibleDuration;
	m_hitFlashTimer = m_hitFlashDuration;
	m_cameraShakeTimer = m_cameraShakeDuration;
	UpdateHpUI();

	if (m_currentHp > 0)
	{
		DebugLog::Warning(
			"Player hit obstacle. HP: " + std::to_string(m_currentHp) + " / " + std::to_string(m_maxHp));
		return true;
	}

	DebugLog::Error("Player HP reached zero. Game Over.");
	GameOver();
	return true;
}

void CookieRunGameManagerComponent::GameOver()
{
	if (m_gameOver)
	{
		return;
	}

	m_gameOver = true;
	m_scoreCollectEffectActive = false;
	s_lastScore = m_score;
	UpdateHpUI();

	if (MainFrame* mainFrame = MainFrame::GetInstance())
	{
		mainFrame->SetTimeScale(m_currentDifficultyTimeScale);
	}

	if (UILabel* scoreLabel = GetScoreLabel())
	{
		scoreLabel->SetScale(m_scoreLabelBaseScale);
	}

	DebugLog::Log("ResultScene requested.");
	SceneChangeAction::RequestSceneChange("ResultScene");
}

int CookieRunGameManagerComponent::GetScore() const
{
	return m_score;
}

int CookieRunGameManagerComponent::GetCurrentHp() const
{
	return m_currentHp;
}

int CookieRunGameManagerComponent::GetMaxHp() const
{
	return m_maxHp;
}

float CookieRunGameManagerComponent::GetScrollSpeed() const
{
	return m_scrollSpeed;
}

bool CookieRunGameManagerComponent::IsGameOver() const
{
	return m_gameOver;
}

bool CookieRunGameManagerComponent::IsScoreCollectEffectActive() const
{
	return m_scoreCollectEffectActive;
}

void CookieRunGameManagerComponent::UpdateScoreUI()
{
	if (GetScoreLabel() == nullptr)
	{
		ResolveFallbackReferences();
	}

	if (UILabel* scoreLabel = GetScoreLabel())
	{
		scoreLabel->SetText(ToWideLabel("Score: ", m_score));
	}
}

void CookieRunGameManagerComponent::UpdateSpeedUI()
{
	if (GetSpeedLabel() == nullptr)
	{
		ResolveFallbackReferences();
	}

	if (UILabel* speedLabel = GetSpeedLabel())
	{
		speedLabel->SetText(ToWideLabel("Speed: ", static_cast<int>(m_scrollSpeed)));
	}
}

void CookieRunGameManagerComponent::UpdateHpUI()
{
	if (GetHpLabel() == nullptr)
	{
		ResolveFallbackReferences();
	}

	if (UILabel* hpLabel = GetHpLabel())
	{
		hpLabel->SetText(ToWideHpLabel(m_currentHp, m_maxHp));
	}
}

int CookieRunGameManagerComponent::GetLastScore()
{
	return s_lastScore;
}

void CookieRunGameManagerComponent::UpdateHitFlashAndCameraShake(float deltaTime)
{
	if (m_hitFlashTimer > 0.0f)
	{
		m_hitFlashTimer -= deltaTime;
		if (m_hitFlashTimer < 0.0f)
		{
			m_hitFlashTimer = 0.0f;
		}
	}

	if (ImageRender* playerImageRender = GetPlayerImageRender())
	{
		if (m_scoreCollectEffectActive)
		{
			playerImageRender->SetColor(kScoreCollectPlayerColor);
		}
		else if (m_hitFlashTimer > 0.0f)
		{
			playerImageRender->SetColor(kHitFlashPlayerColor);
		}
		else if (m_startInvincibleTimer > 0.0f)
		{
			playerImageRender->SetColor(kStartInvinciblePlayerColor);
		}
		else
		{
			playerImageRender->SetColor(kDefaultPlayerColor);
		}
	}

	if (Camera* camera = Camera::GetInstance())
	{
		float finalRotationX = m_baseCameraRotation.x;
		float finalRotationY = m_baseCameraRotation.y;

		if (m_cameraShakeTimer > 0.0f)
		{
			m_cameraShakeTimer -= deltaTime;
			if (m_cameraShakeTimer < 0.0f)
			{
				m_cameraShakeTimer = 0.0f;
			}

			const float normalizedTime =
				(m_cameraShakeDuration > 0.0f) ? (m_cameraShakeTimer / m_cameraShakeDuration) : 0.0f;
			const float damping = normalizedTime * normalizedTime;
			const float shakeX = sinf(m_elapsedTime * 38.0f) * m_cameraShakeStrengthX * damping;
			const float shakeY = cosf(m_elapsedTime * 44.0f) * m_cameraShakeStrengthY * damping;
			finalRotationX += shakeX;
			finalRotationY += shakeY;
		}

		if (m_scoreCollectEffectActive && m_scoreCollectEffectDuration > 0.0f)
		{
			float normalizedTime = m_scoreCollectEffectTimer / m_scoreCollectEffectDuration;
			if (normalizedTime > 1.0f)
			{
				normalizedTime = 1.0f;
			}

			const float wave = sinf(normalizedTime * D3DX_PI);
			finalRotationX += m_scoreCollectCameraTiltX * wave;
			finalRotationY += m_scoreCollectCameraTiltY * wave;
		}

		camera->SetRotation(finalRotationX, finalRotationY, m_baseCameraRotation.z);
	}
}

void CookieRunGameManagerComponent::UpdateDifficultyTimeScale(float deltaTime, float unscaledDeltaTime)
{
	(void)deltaTime;

	if (m_scoreCollectEffectActive)
	{
		return;
	}

	if (m_timeScaleIncreaseInterval <= 0.0f || m_timeScaleIncreaseAmount <= 0.0f)
	{
		return;
	}

	m_timeScaleIncreaseTimer += unscaledDeltaTime;
	while (m_timeScaleIncreaseTimer >= m_timeScaleIncreaseInterval)
	{
		m_timeScaleIncreaseTimer -= m_timeScaleIncreaseInterval;
		m_currentDifficultyTimeScale += m_timeScaleIncreaseAmount;
		if (m_currentDifficultyTimeScale > m_maxDifficultyTimeScale)
		{
			m_currentDifficultyTimeScale = m_maxDifficultyTimeScale;
		}

		if (MainFrame* mainFrame = MainFrame::GetInstance())
		{
			mainFrame->SetTimeScale(m_currentDifficultyTimeScale);
		}

		DebugLog::Log("Time scale increased.");
	}
}

void CookieRunGameManagerComponent::UpdateScoreCollectEffect(float unscaledDeltaTime)
{
	if (!m_scoreCollectEffectActive)
	{
		return;
	}

	if (m_player == nullptr || m_scoreCollectEffectDuration <= 0.0f)
	{
		m_scoreCollectEffectActive = false;
		if (ImageRender* playerImageRender = GetPlayerImageRender())
		{
			playerImageRender->SetColor(m_hitFlashTimer > 0.0f ? kHitFlashPlayerColor : kDefaultPlayerColor);
		}
		if (MainFrame* mainFrame = MainFrame::GetInstance())
		{
			mainFrame->SetTimeScale(m_scoreCollectEffectSavedTimeScale);
		}
		return;
	}

	m_scoreCollectEffectTimer += unscaledDeltaTime;
	float normalizedTime = m_scoreCollectEffectTimer / m_scoreCollectEffectDuration;
	if (normalizedTime > 1.0f)
	{
		normalizedTime = 1.0f;
	}

	const float fullRotation = D3DX_PI * 2.0f;
	m_player->SetAngleY(m_scoreCollectEffectStartAngleY + (fullRotation * normalizedTime));

	if (ImageRender* playerImageRender = GetPlayerImageRender())
	{
		playerImageRender->SetColor(kScoreCollectPlayerColor);
	}

	if (UILabel* scoreLabel = GetScoreLabel())
	{
		const float wave = sinf(normalizedTime * D3DX_PI);
		scoreLabel->SetScale(m_scoreLabelBaseScale + (m_scoreCollectLabelPulseScale * wave));
	}

	if (normalizedTime >= 1.0f)
	{
		m_player->SetAngleY(m_scoreCollectEffectStartAngleY);
		m_scoreCollectEffectActive = false;

		if (UILabel* scoreLabel = GetScoreLabel())
		{
			scoreLabel->SetScale(m_scoreLabelBaseScale);
		}

		if (ImageRender* playerImageRender = GetPlayerImageRender())
		{
			playerImageRender->SetColor(m_hitFlashTimer > 0.0f ? kHitFlashPlayerColor : kDefaultPlayerColor);
		}

		if (MainFrame* mainFrame = MainFrame::GetInstance())
		{
			mainFrame->SetTimeScale(m_scoreCollectEffectSavedTimeScale);
		}
	}
}

const char* CookieRunGameManagerComponent::GetInspectorName() const
{
	return "CookieRun Game Manager";
}

void CookieRunGameManagerComponent::DrawInspector()
{
	SyncImGuiContextForCurrentModule();
	ImGui::Text("Score: %d", m_score);
	ImGui::Text("HP: %d / %d", m_currentHp, m_maxHp);
	ImGui::DragInt("Max HP", &m_maxHp, 1.0f, 1, 99);
	ImGui::DragFloat("Scroll Speed", &m_scrollSpeed, 1.0f, 0.0f, 5000.0f);
	ImGui::DragFloat("Speed Increase Interval", &m_speedIncreaseInterval, 0.1f, 0.1f, 60.0f);
	ImGui::DragFloat("Speed Increase Amount", &m_speedIncreaseAmount, 1.0f, 0.0f, 1000.0f);
	ImGui::DragFloat("Damage Invincible Duration", &m_damageInvincibleDuration, 0.05f, 0.0f, 10.0f);
	ImGui::DragFloat("Start Invincible Duration", &m_startInvincibleDuration, 0.05f, 0.0f, 10.0f);
	ImGui::DragFloat("Hit Flash Duration", &m_hitFlashDuration, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat("Camera Shake Duration", &m_cameraShakeDuration, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat("Camera Shake X", &m_cameraShakeStrengthX, 0.001f, 0.0f, 0.5f);
	ImGui::DragFloat("Camera Shake Y", &m_cameraShakeStrengthY, 0.001f, 0.0f, 0.5f);
	ImGui::DragFloat("TimeScale Increase Interval", &m_timeScaleIncreaseInterval, 0.1f, 0.1f, 60.0f);
	ImGui::DragFloat("TimeScale Increase Amount", &m_timeScaleIncreaseAmount, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat("Max Difficulty TimeScale", &m_maxDifficultyTimeScale, 0.01f, 1.0f, 10.0f);
	ImGui::DragFloat("Score Effect Duration", &m_scoreCollectEffectDuration, 0.01f, 0.01f, 2.0f);
	ImGui::DragFloat("Score Label Pulse Scale", &m_scoreCollectLabelPulseScale, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat("Score Camera Tilt X", &m_scoreCollectCameraTiltX, 0.0001f, 0.0f, 0.1f);
	ImGui::DragFloat("Score Camera Tilt Y", &m_scoreCollectCameraTiltY, 0.0001f, 0.0f, 0.1f);
	ImGui::Text("Current TimeScale: %.2f", m_currentDifficultyTimeScale);
	ImGui::Text("Game Over: %s", m_gameOver ? "true" : "false");
}

const char* CookieRunGameManagerComponent::GetSerializableType() const
{
	return kComponentType;
}

std::string CookieRunGameManagerComponent::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"maxHp\": " << m_maxHp << ", ";
	oss << "\"scrollSpeed\": " << m_scrollSpeed << ", ";
	oss << "\"speedIncreaseInterval\": " << m_speedIncreaseInterval << ", ";
	oss << "\"speedIncreaseAmount\": " << m_speedIncreaseAmount << ", ";
	oss << "\"damageInvincibleDuration\": " << m_damageInvincibleDuration << ", ";
	oss << "\"startInvincibleDuration\": " << m_startInvincibleDuration << ", ";
	oss << "\"hitFlashDuration\": " << m_hitFlashDuration << ", ";
	oss << "\"cameraShakeDuration\": " << m_cameraShakeDuration << ", ";
	oss << "\"cameraShakeStrengthX\": " << m_cameraShakeStrengthX << ", ";
	oss << "\"cameraShakeStrengthY\": " << m_cameraShakeStrengthY << ", ";
	oss << "\"timeScaleIncreaseInterval\": " << m_timeScaleIncreaseInterval << ", ";
	oss << "\"timeScaleIncreaseAmount\": " << m_timeScaleIncreaseAmount << ", ";
	oss << "\"maxDifficultyTimeScale\": " << m_maxDifficultyTimeScale << ", ";
	oss << "\"scoreCollectEffectDuration\": " << m_scoreCollectEffectDuration << ", ";
	oss << "\"scoreCollectLabelPulseScale\": " << m_scoreCollectLabelPulseScale << ", ";
	oss << "\"scoreCollectCameraTiltX\": " << m_scoreCollectCameraTiltX << ", ";
	oss << "\"scoreCollectCameraTiltY\": " << m_scoreCollectCameraTiltY;
	oss << " }";
	return oss.str();
}

bool CookieRunGameManagerComponent::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadInt(componentJson, "maxHp", m_maxHp);
	SceneJson::ReadFloat(componentJson, "scrollSpeed", m_scrollSpeed);
	SceneJson::ReadFloat(componentJson, "speedIncreaseInterval", m_speedIncreaseInterval);
	SceneJson::ReadFloat(componentJson, "speedIncreaseAmount", m_speedIncreaseAmount);
	SceneJson::ReadFloat(componentJson, "damageInvincibleDuration", m_damageInvincibleDuration);
	SceneJson::ReadFloat(componentJson, "startInvincibleDuration", m_startInvincibleDuration);
	SceneJson::ReadFloat(componentJson, "hitFlashDuration", m_hitFlashDuration);
	SceneJson::ReadFloat(componentJson, "cameraShakeDuration", m_cameraShakeDuration);
	SceneJson::ReadFloat(componentJson, "cameraShakeStrengthX", m_cameraShakeStrengthX);
	SceneJson::ReadFloat(componentJson, "cameraShakeStrengthY", m_cameraShakeStrengthY);
	SceneJson::ReadFloat(componentJson, "timeScaleIncreaseInterval", m_timeScaleIncreaseInterval);
	SceneJson::ReadFloat(componentJson, "timeScaleIncreaseAmount", m_timeScaleIncreaseAmount);
	SceneJson::ReadFloat(componentJson, "maxDifficultyTimeScale", m_maxDifficultyTimeScale);
	SceneJson::ReadFloat(componentJson, "scoreCollectEffectDuration", m_scoreCollectEffectDuration);
	SceneJson::ReadFloat(componentJson, "scoreCollectLabelPulseScale", m_scoreCollectLabelPulseScale);
	SceneJson::ReadFloat(componentJson, "scoreCollectCameraTiltX", m_scoreCollectCameraTiltX);
	SceneJson::ReadFloat(componentJson, "scoreCollectCameraTiltY", m_scoreCollectCameraTiltY);
	return true;
}

void CookieRunGameManagerComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	registry.RegisterGameObjectRef("Player GameObject", &m_player);
	registry.RegisterComponentRef("ScoreLabel Component", &m_scoreLabelComponent, "UILabel");
	registry.RegisterComponentRef("SpeedLabel Component", &m_speedLabelComponent, "UILabel");
	registry.RegisterComponentRef("HPLabel Component", &m_hpLabelComponent, "UILabel");
	registry.RegisterComponentRef("BackgroundPool Component", &m_backgroundPoolComponent, "LoopObjectPoolComponent");
	registry.RegisterComponentRef("LandPool Component", &m_landPoolComponent, "LoopObjectPoolComponent");
	registry.RegisterComponentRef("ItemPool Component", &m_itemPoolComponent, "SpawnObjectPoolComponent");
	registry.RegisterComponentRef("ObstaclePool Component", &m_obstaclePoolComponent, "SpawnObjectPoolComponent");
}

void CookieRunGameManagerComponent::ResolveFallbackReferences()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return;
	}

	if (m_player == nullptr)
	{
		m_player = objectManager->FindObjectByName("Player");
	}

	if (m_scoreLabelComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("ScoreLabel"))
		{
			m_scoreLabelComponent = object->GetComponent<UILabel>();
		}
	}

	if (m_speedLabelComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("SpeedLabel"))
		{
			m_speedLabelComponent = object->GetComponent<UILabel>();
		}
	}

	if (m_hpLabelComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("HPLabel"))
		{
			m_hpLabelComponent = object->GetComponent<UILabel>();
		}
	}

	if (m_backgroundPoolComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("BackgroundPool"))
		{
			m_backgroundPoolComponent = object->GetComponent<LoopObjectPoolComponent>();
		}
	}

	if (m_landPoolComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("LandPool"))
		{
			m_landPoolComponent = object->GetComponent<LoopObjectPoolComponent>();
		}
	}

	if (m_itemPoolComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("ItemPool"))
		{
			m_itemPoolComponent = object->GetComponent<SpawnObjectPoolComponent>();
		}
	}

	if (m_obstaclePoolComponent == nullptr)
	{
		if (GameObject* object = objectManager->FindObjectByName("ObstaclePool"))
		{
			m_obstaclePoolComponent = object->GetComponent<SpawnObjectPoolComponent>();
		}
	}
}

void CookieRunGameManagerComponent::SanitizeCachedReferences()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		m_player = nullptr;
		return;
	}

	if (m_player != nullptr && !objectManager->IsTrackedObjectPointer(m_player))
	{
		m_player = nullptr;
	}
}

UILabel* CookieRunGameManagerComponent::GetScoreLabel() const
{
	return dynamic_cast<UILabel*>(m_scoreLabelComponent);
}

UILabel* CookieRunGameManagerComponent::GetSpeedLabel() const
{
	return dynamic_cast<UILabel*>(m_speedLabelComponent);
}

UILabel* CookieRunGameManagerComponent::GetHpLabel() const
{
	return dynamic_cast<UILabel*>(m_hpLabelComponent);
}

ObjectPoolComponent* CookieRunGameManagerComponent::GetBackgroundPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_backgroundPoolComponent);
}

ObjectPoolComponent* CookieRunGameManagerComponent::GetLandPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_landPoolComponent);
}

ObjectPoolComponent* CookieRunGameManagerComponent::GetItemPool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_itemPoolComponent);
}

ObjectPoolComponent* CookieRunGameManagerComponent::GetObstaclePool() const
{
	return dynamic_cast<ObjectPoolComponent*>(m_obstaclePoolComponent);
}

ImageRender* CookieRunGameManagerComponent::GetPlayerImageRender() const
{
	if (m_player == nullptr)
	{
		return nullptr;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr || !objectManager->IsTrackedObjectPointer(m_player))
	{
		return nullptr;
	}

	return m_player->GetComponent<ImageRender>();
}
