#pragma once

class UIButton;

class UIActionRegistry
{
private:
	static std::map<std::string, std::function<void()>>& GetActions();

public:
	static void RegisterAction(const std::string& actionKey, std::function<void()> action);
	static void UnregisterAction(const std::string& actionKey);
	static void Clear();
	static bool Bind(UIButton* button, const std::string& actionKey);
};
