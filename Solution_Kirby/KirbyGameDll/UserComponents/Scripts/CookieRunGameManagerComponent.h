#pragma once

#include "Component.h"

class UILabel;
class ObjectPoolComponent;
class ImageRender;

class CookieRunGameManagerComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunGameManagerComponent";

	CookieRunGameManagerComponent() = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void AddScore(int amount);
	bool DamagePlayer(int amount);
	void GameOver();

	int GetScore() const;
	int GetCurrentHp() const;
	int GetMaxHp() const;
	float GetScrollSpeed() const;
	bool IsGameOver() const;
	bool IsScoreCollectEffectActive() const;

	void UpdateScoreUI();
	void UpdateSpeedUI();
	void UpdateHpUI();

	static int GetLastScore();

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void RegisterReferenceFields(ReferenceFieldRegistry& registry) override;

private:
	void ResolveFallbackReferences();
	void SanitizeCachedReferences();
	void UpdateHitFlashAndCameraShake(float deltaTime);
	void UpdateDifficultyTimeScale(float deltaTime, float unscaledDeltaTime);
	void UpdateScoreCollectEffect(float unscaledDeltaTime);
	ImageRender* GetPlayerImageRender() const;
	UILabel* GetScoreLabel() const;
	UILabel* GetSpeedLabel() const;
	UILabel* GetHpLabel() const;
	ObjectPoolComponent* GetBackgroundPool() const;
	ObjectPoolComponent* GetLandPool() const;
	ObjectPoolComponent* GetItemPool() const;
	ObjectPoolComponent* GetObstaclePool() const;

	GameObject* m_player = nullptr;
	Component* m_scoreLabelComponent = nullptr;
	Component* m_speedLabelComponent = nullptr;
	Component* m_hpLabelComponent = nullptr;
	Component* m_backgroundPoolComponent = nullptr;
	Component* m_landPoolComponent = nullptr;
	Component* m_itemPoolComponent = nullptr;
	Component* m_obstaclePoolComponent = nullptr;

	int m_score = 0;
	int m_maxHp = 3;
	int m_currentHp = 3;
	float m_scrollSpeed = 220.0f;
	float m_speedIncreaseInterval = 5.0f;
	float m_speedIncreaseAmount = 30.0f;
	float m_elapsedTime = 0.0f;
	float m_speedIncreaseTimer = 0.0f;
	float m_timeScaleIncreaseTimer = 0.0f;
	float m_damageInvincibleDuration = 0.8f;
	float m_damageInvincibleTimer = 0.0f;
	float m_hitFlashDuration = 0.14f;
	float m_hitFlashTimer = 0.0f;
	float m_cameraShakeDuration = 0.16f;
	float m_cameraShakeTimer = 0.0f;
	float m_cameraShakeStrengthX = 0.0045f;
	float m_cameraShakeStrengthY = 0.0065f;
	float m_timeScaleIncreaseInterval = 6.0f;
	float m_timeScaleIncreaseAmount = 0.05f;
	float m_maxDifficultyTimeScale = 2.0f;
	float m_currentDifficultyTimeScale = 1.0f;
	float m_scoreCollectEffectDuration = 0.22f;
	float m_scoreCollectEffectTimer = 0.0f;
	float m_scoreCollectEffectSavedTimeScale = 1.0f;
	float m_scoreCollectEffectStartAngleY = 0.0f;
	float m_scoreCollectLabelPulseScale = 0.3f;
	float m_scoreCollectCameraTiltX = 0.0015f;
	float m_scoreCollectCameraTiltY = 0.0020f;
	float m_scoreLabelBaseScale = 1.0f;
	D3DXVECTOR3 m_baseCameraRotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	bool m_gameOver = false;
	bool m_scoreCollectEffectActive = false;

	static int s_lastScore;
};
