#include "pch.h"
#include "LoopObjectPoolComponent.h"

void LoopObjectPoolComponent::Initialize()
{
	SetPoolMode(true);
}

const char* LoopObjectPoolComponent::GetInspectorName() const
{
	return "Loop Object Pool";
}

void LoopObjectPoolComponent::DrawInspector()
{
	DrawSharedInspector(false);
}

const char* LoopObjectPoolComponent::GetSerializableType() const
{
	return kComponentType;
}

bool LoopObjectPoolComponent::Deserialize(const std::string& componentJson)
{
	if (!ObjectPoolComponent::Deserialize(componentJson))
	{
		return false;
	}

	SetPoolMode(true);
	return true;
}
