#include "BuildLoopCls.h"
#include "../BuildCommon/AppLaunchDesc.h"
#include "../BuildCommon/AppBootstrap.h"
#include "../include/PluginManager.h"
#include "../include/plugin.h"
#include "../EngineFrameworkDll/pch.h"


void BuildLoopCls::Init(HINSTANCE hInstance)
{
	std::string dllfilename = "KirbyGameDll.dll";

	// 현재 실행 폴더 경로
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, path, MAX_PATH);
	std::string executepath = ConvertToString(path);
	executepath = executepath.substr(0, executepath.find_last_of("\\/"));
	PluginManager::GetInstance()->DirectLoadPlugin(executepath + "\\" + dllfilename);

	auto plugincls = PluginManager::GetInstance()->GetPlugin(0);
	if (plugincls == nullptr)
	{
		return;
	}

	const AppLaunchDesc launchDesc = BuildGameLaunchDesc();
	if (!plugincls->RegisterGameContent(hInstance, launchDesc))
	{
		return;
	}

	if (!BootstrapMainFrame(hInstance, launchDesc))
	{
		return;
	}
}

bool BuildLoopCls::Update()
{
	return UpdateMainFrame();
}

void BuildLoopCls::Release()
{
	ShutdownMainFrame();

	auto size = PluginManager::GetInstance()->GetNumPlugins();
	for (size_t i = 0; i < size; i++)
	{
		PluginManager::GetInstance()->GetPlugin(i)->ReleaseGameContent();
	}
	PluginManager::GetInstance()->UnloadAllPlugins();
}
