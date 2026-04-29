#include "pch.h"
#include "UIActionRegistry.h"
#include "UIButton.h"

std::map<std::string, std::function<void()>>& UIActionRegistry::GetActions()
{
	static std::map<std::string, std::function<void()>> actions;
	return actions;
}

void UIActionRegistry::RegisterAction(const std::string& actionKey, std::function<void()> action)
{
	if (actionKey.empty())
		return;

	GetActions()[actionKey] = action;
}

void UIActionRegistry::UnregisterAction(const std::string& actionKey)
{
	GetActions().erase(actionKey);
}

void UIActionRegistry::Clear()
{
	GetActions().clear();
}

bool UIActionRegistry::Bind(UIButton* button, const std::string& actionKey)
{
	if (button == nullptr)
		return false;

	if (actionKey.empty())
	{
		button->SetOnClick(nullptr);
		return false;
	}

	std::map<std::string, std::function<void()>>::iterator itr = GetActions().find(actionKey);
	if (itr == GetActions().end())
	{
		button->SetOnClick(nullptr);
		std::cout << "UIActionRegistry missing action: " << actionKey << std::endl;
		return false;
	}

	button->SetOnClick(itr->second);
	return true;
}
