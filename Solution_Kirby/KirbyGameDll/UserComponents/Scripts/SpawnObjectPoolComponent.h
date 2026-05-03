#pragma once

#include "ObjectPoolComponent.h"

class SpawnObjectPoolComponent : public ObjectPoolComponent
{
public:
	static constexpr const char* kComponentType = "SpawnObjectPoolComponent";

	SpawnObjectPoolComponent() = default;

	void Initialize() override;
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	bool Deserialize(const std::string& componentJson) override;
};
