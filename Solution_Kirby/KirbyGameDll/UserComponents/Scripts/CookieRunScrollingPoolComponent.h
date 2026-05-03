#pragma once

#include "Component.h"

class CookieRunGameManagerComponent;
class ObjectPoolComponent;

class CookieRunScrollingPoolComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunScrollingPoolComponent";

	CookieRunScrollingPoolComponent() = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void RegisterReferenceFields(ReferenceFieldRegistry& registry) override;

private:
	void ResolveFallbackReferences();
	ObjectPoolComponent* GetPool() const;
	CookieRunGameManagerComponent* GetGameManager() const;

	Component* m_poolComponent = nullptr;
	Component* m_gameManagerComponent = nullptr;
	float m_scrollSpeed = 220.0f;
	float m_leftBound = -900.0f;
	bool m_useGameManagerSpeed = true;
};
