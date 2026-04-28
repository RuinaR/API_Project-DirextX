#include "pch.h"
#include "ComponentFactory.h"
#include "SceneJsonUtility.h"
#include "ImageRender.h"
#include "AnimationRender.h"
#include "FBXRender.h"
#include "BoxCollider.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UILabel.h"

ComponentFactory& ComponentFactory::GetInstance()
{
	static ComponentFactory instance;
	return instance;
}

bool ComponentFactory::Register(const std::string& typeName, const std::string& displayName, CreateFunction createFn)
{
	return Register(typeName, displayName, "General", true, createFn);
}

bool ComponentFactory::Register(const std::string& typeName, const std::string& displayName, const std::string& category, bool canAddFromEditor, CreateFunction createFn)
{
	if (typeName.empty() || createFn == nullptr || IsRegistered(typeName))
	{
		return false;
	}

	ComponentInfo info;
	info.typeName = typeName;
	info.displayName = displayName.empty() ? typeName : displayName;
	info.category = category.empty() ? "General" : category;
	info.canAddFromEditor = canAddFromEditor;
	info.createFn = createFn;
	m_components.push_back(info);
	return true;
}

bool ComponentFactory::IsRegistered(const std::string& typeName) const
{
	for (std::vector<ComponentInfo>::const_iterator itr = m_components.begin(); itr != m_components.end(); itr++)
	{
		if (itr->typeName == typeName)
		{
			return true;
		}
	}
	return false;
}

Component* ComponentFactory::Create(const std::string& typeName, const std::string& componentJson) const
{
	for (std::vector<ComponentInfo>::const_iterator itr = m_components.begin(); itr != m_components.end(); itr++)
	{
		if (itr->typeName == typeName)
		{
			return itr->createFn(componentJson);
		}
	}
	return nullptr;
}

const std::vector<ComponentFactory::ComponentInfo>& ComponentFactory::GetRegisteredComponents() const
{
	return m_components;
}

void ComponentFactory::Clear()
{
	m_components.clear();
}

void RegisterEngineComponents()
{
	ComponentFactory& factory = ComponentFactory::GetInstance();

	factory.Register("ImageRender", "Image Render", "Rendering", true, [](const std::string&) -> Component*
		{
			return new ImageRender(nullptr);
		});
	factory.Register("AnimationRender", "Animation Render", "Rendering", true, [](const std::string&) -> Component*
		{
			return new AnimationRender(Animation());
		});
	factory.Register("FBXRender", "FBX Render", "Rendering", true, [](const std::string& componentJson) -> Component*
		{
			std::string fbxPath;
			SceneJson::ReadString(componentJson, "fbxPath", fbxPath);
			return new FBXRender(fbxPath);
		});
	factory.Register("BoxCollider", "Box Collider", "Physics", true, [](const std::string&) -> Component*
		{
			return new BoxCollider(b2_staticBody);
		});
	factory.Register("UIImage", "UI Image", "UI", true, [](const std::string&) -> Component*
		{
			return new UIImage();
		});
	factory.Register("UIButton", "UI Button", "UI", true, [](const std::string&) -> Component*
		{
			return new UIButton();
		});
	factory.Register("UILabel", "UI Label", "UI", true, [](const std::string&) -> Component*
		{
			return new UILabel();
		});
}
