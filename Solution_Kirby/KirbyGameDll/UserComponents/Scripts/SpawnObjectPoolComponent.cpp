#include "pch.h"
#include "SpawnObjectPoolComponent.h"

void SpawnObjectPoolComponent::Initialize()
{
	SetPoolMode(false);
}

const char* SpawnObjectPoolComponent::GetInspectorName() const
{
	return "Spawn Object Pool";
}

void SpawnObjectPoolComponent::DrawInspector()
{
	DrawSharedInspector(false);
}

const char* SpawnObjectPoolComponent::GetSerializableType() const
{
	return kComponentType;
}

bool SpawnObjectPoolComponent::Deserialize(const std::string& componentJson)
{
	if (!ObjectPoolComponent::Deserialize(componentJson))
	{
		return false;
	}

	SetPoolMode(false);
	return true;
}
