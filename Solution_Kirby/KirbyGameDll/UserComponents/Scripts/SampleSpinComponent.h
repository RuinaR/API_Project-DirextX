#pragma once

#include "Component.h"

class SampleSpinComponent : public Component
{
public:
	static constexpr const char* kComponentType = "SampleSpinComponent";

	SampleSpinComponent() = default;
	~SampleSpinComponent() override = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
	void ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap) override;

private:
	bool m_enabled = true;
	float m_spinSpeed = 1.0f;
	bool m_useYAxis = true;
	bool m_useZAxis = false;
	GameObject* m_targetObject = nullptr;
	int m_targetObjectId = -1;
};
