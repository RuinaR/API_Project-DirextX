#pragma once

#include "ObjectPoolComponent.h"

class LoopObjectPoolComponent : public ObjectPoolComponent
{
public:
	static constexpr const char* kComponentType = "LoopObjectPoolComponent";

	LoopObjectPoolComponent() = default;

	void Initialize() override;
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	bool Deserialize(const std::string& componentJson) override;
};
