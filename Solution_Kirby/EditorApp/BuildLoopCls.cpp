#include "BuildLoopCls.h"
#include "../BuildCommon/AppLaunchDesc.h"
#include "../BuildCommon/AppBootstrap.h"
#include "../EngineFrameworkDll/Editor/EditorRenderFacade.h"
#include "../EngineFrameworkDll/Editor/EditorSceneWorkflow.h"
#include "../EngineFrameworkDll/DebugLog.h"
#include "../EngineFrameworkDll/RenderManager.h"
#include "../EngineFrameworkDll/Scene/SceneDataManager.h"
#include "../EngineFrameworkDll/WindowFrame.h"
#include "../include/PluginManager.h"
#include "../include/plugin.h"
#include "../EngineFrameworkDll/pch.h"


void BuildLoopCls::Init(HINSTANCE hInstance)
{
	std::string dllfilename = "KirbyGameDll.dll";

	// 지금 실행 중인 exe 폴더를 구해서 DLL 경로를 만든다.
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

	const AppLaunchDesc launchDesc = BuildEditorLaunchDesc();
	if (!plugincls->RegisterGameContent(hInstance, launchDesc))
	{
		return;
	}

	DebugLog::Clear();
	DebugLog::Log("EditorApp initialized.");

	WindowFrame::SetSceneSnapshotCallback(
		[](const std::string& sceneName)
		{
			EditorSceneWorkflow::CaptureSceneSnapshot(sceneName);
		});
	RenderManager::SetEditorOverlayCallback(
		[](RenderManager& renderManager)
		{
			EditorRenderFacade::DrawOverlay(renderManager);
		});

	if (!BootstrapMainFrame(hInstance, launchDesc))
	{
		return;
	}

	if (launchDesc.createNewSceneIfMissing &&
		!SceneDataManager::Exists(launchDesc.startupSceneName))
	{
		EditorSceneWorkflow::CreateNewScene(launchDesc.startupSceneName);
	}
}

bool BuildLoopCls::Update()
{
	return UpdateMainFrame();
}

void BuildLoopCls::Release()
{
	ShutdownMainFrame();
	RenderManager::ClearEditorOverlayCallback();
	WindowFrame::ClearSceneSnapshotCallback();

	auto size = PluginManager::GetInstance()->GetNumPlugins();
	for (size_t i = 0; i < size; i++)
	{
		PluginManager::GetInstance()->GetPlugin(i)->ReleaseGameContent();
	}
	PluginManager::GetInstance()->UnloadAllPlugins();
}
