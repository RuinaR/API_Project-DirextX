#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Component.h"

class ComponentFactory
{
public:
	using CreateFunction = std::function<Component*(const std::string& componentJson)>;

	struct ComponentInfo
	{
		std::string typeName;
		std::string displayName;
		std::string category;
		bool canAddFromEditor = true;
		CreateFunction createFn;
	};

	static ComponentFactory& GetInstance();

	bool Register(const std::string& typeName, const std::string& displayName, CreateFunction createFn);
	bool Register(const std::string& typeName, const std::string& displayName, const std::string& category, bool canAddFromEditor, CreateFunction createFn);
	bool IsRegistered(const std::string& typeName) const;
	Component* Create(const std::string& typeName, const std::string& componentJson) const;
	const std::vector<ComponentInfo>& GetRegisteredComponents() const;
	void Clear();

private:
	std::vector<ComponentInfo> m_components;
};

void RegisterEngineComponents();
