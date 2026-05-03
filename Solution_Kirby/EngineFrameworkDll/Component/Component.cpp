#include "pch.h"
#include "Component.h"
Component::Component()
{
}

Component::~Component()
{
}

void Component::InitGameObj(GameObject* obj)
{
	m_gameObj = obj;
}

GameObject* Component::GetGameObject()
{
	return this->m_gameObj;
}

const char* Component::GetInspectorName() const
{
	return typeid(*this).name();
}

void Component::DrawInspector()
{
}

const char* Component::GetSerializableType() const
{
	return "";
}

std::string Component::Serialize() const
{
	return "{}";
}

bool Component::Deserialize(const std::string& componentJson)
{
	return true;
}

void Component::ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap)
{
	(void)objectMap;
}

void Component::OnDeviceLost()
{
}

void Component::OnDeviceReset()
{
}
