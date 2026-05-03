#pragma once

#include "Component.h"

class CookieRunRotateYComponent : public Component
{
public:
	static constexpr const char* kComponentType = "CookieRunRotateYComponent";

	CookieRunRotateYComponent() = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

private:
	float m_rotationSpeed = 0.7f;
};
