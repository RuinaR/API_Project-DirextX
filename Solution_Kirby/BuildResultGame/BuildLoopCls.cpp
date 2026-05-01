#include "BuildLoopCls.h"
#include "../include/PluginManager.h"
#include "../include/plugin.h"
#include "../EngineFramework/pch.h"


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
	plugincls->Initialize(hInstance, RenderType::Game);
}

bool BuildLoopCls::Update()
{
	auto size = PluginManager::GetInstance()->GetNumPlugins();
	for (size_t i = 0; i < size; i++)
	{
		if (!PluginManager::GetInstance()->GetPlugin(i)->AllUpdate())
			return false;
	}
	return true;
}

void BuildLoopCls::Release()
{
	auto size = PluginManager::GetInstance()->GetNumPlugins();
	for (size_t i = 0; i < size; i++)
	{
		PluginManager::GetInstance()->GetPlugin(i)->AllRelease();
	}
	PluginManager::GetInstance()->UnloadAllPlugins();
}
