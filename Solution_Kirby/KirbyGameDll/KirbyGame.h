#pragma once
#include "../include/plugin.h"
class KirbyGame : public IPlugin
{
public:
	KirbyGame(PluginManager& p_mgr);

	virtual bool RegisterGameContent(HINSTANCE hInst, const AppLaunchDesc& launchDesc);
	virtual const std::string GetName();

	void ReleaseGameContent() override;

private:
	PluginManager& m_hManager;
	std::string m_strName;
};
