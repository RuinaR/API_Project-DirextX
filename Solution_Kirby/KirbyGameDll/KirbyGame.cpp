#include "pch.h"
#include "KirbyGame.h"
#include "../include/PluginManager.h"

#include "ComponentFactory.h"
#include "UserComponents/GameComponentRegistry.h"
#include "UIActionRegistry.h"
#include "UserActions/GameActionRegistry.h"


KirbyGame::KirbyGame(PluginManager& p_mgr) :
	m_hManager(p_mgr),
	m_strName("KirbyGame")
{

}

bool KirbyGame::RegisterGameContent(HINSTANCE hInst, const AppLaunchDesc& launchDesc)
{
	UNREFERENCED_PARAMETER(hInst);

	RegisterEngineComponents();
	RegisterGameComponents(ComponentFactory::GetInstance());
	RegisterGameUIActions(launchDesc.renderType);

	return true;
}

const std::string KirbyGame::GetName()
{
	return m_strName;
}

void KirbyGame::ReleaseGameContent()
{
	// Clear DLL-registered callbacks/factories before the DLL unloads.
	UIActionRegistry::Clear();
	ComponentFactory::GetInstance().Clear();
}


PLUGINDECL IPlugin* CreatePlugin(PluginManager& mgr)
{
	return new KirbyGame(mgr);
}
