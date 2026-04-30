#pragma once
#include <Component.h>

class FBXRotateObj :public Component
{
public:
	static constexpr const char* kComponentType = "FBXRotateObj";

private:
	float m_rotateSpeed = 1.0f;
	bool m_rotateX = false;
	bool m_rotateY = true;
	bool m_rotateZ = false;

public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};

