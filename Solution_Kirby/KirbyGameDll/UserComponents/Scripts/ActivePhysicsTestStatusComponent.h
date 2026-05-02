#pragma once

#include "Component.h"

class ActivePhysicsTestStatusComponent : public Component
{
public:
	static constexpr const char* kComponentType = "ActivePhysicsTestStatusComponent";

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

	int GetCollisionCount() const;
	int GetTriggerCount() const;
	int GetEnableCount() const;
	int GetDisableCount() const;
	int GetCollisionEnterCount() const;
	int GetTriggerEnterCount() const;
	const char* GetLastEventText() const;
	std::string GetRecentEventsText() const;

protected:
	void CollisionEnter(Collider2D* other) override;
	void CollisionExit(Collider2D* other) override;
	void TriggerEnter(Collider2D* other) override;
	void TriggerExit(Collider2D* other) override;
	void Enable() override;
	void Disable() override;
	void LBtnDown() override;
	void LBtnUp() override;
	void RBtnDown() override;
	void RBtnUp() override;
	void MouseHoverEnter() override;
	void MouseHoverExit() override;

private:
	void PushEvent(const char* text);

	int m_activeCollisionCount = 0;
	int m_activeTriggerCount = 0;
	int m_enableCount = 0;
	int m_disableCount = 0;
	int m_collisionEnterCount = 0;
	int m_triggerEnterCount = 0;
	std::string m_lastEvent = "None";
	vector<std::string> m_recentEvents;
};
