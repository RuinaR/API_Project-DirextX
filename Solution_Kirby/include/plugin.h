#ifndef IPLUGIN_H_
#define IPLUGIN_H_

#include <string>
#include <wtypes.h>
#include "AppLaunchDesc.h"

#ifdef PLUGIN_EXPORTS
#define PLUGINDECL __declspec(dllexport)
#else
#define PLUGINDECL __declspec(dllimport)
#endif

class PluginManager;

class IPlugin
{
public:
	virtual ~IPlugin() {};

	virtual bool RegisterGameContent(HINSTANCE hInst, const AppLaunchDesc& launchDesc) = 0;
	virtual const std::string GetName() = 0;

	virtual void ReleaseGameContent() {}
};

typedef IPlugin* (*CREATEPLUGIN)(PluginManager &mgr);
// 플러그인 객체를 만들어 주는 함수
extern "C" PLUGINDECL  IPlugin* CreatePlugin(PluginManager &mgr);


#endif  // IPLUGIN_H_
