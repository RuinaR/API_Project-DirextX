#ifndef IPLUGIN_H_
#define IPLUGIN_H_

#include <string>
#include <wtypes.h>
#include "../EngineFramework/pch.h"

#ifdef PLUGIN_EXPORTS
#define PLUGINDECL __declspec(dllexport)
#else
#define PLUGINDECL __declspec(dllimport)
#endif

class PluginManager;
class GameObject;


//
class IPlugin 
{
public:
	virtual ~IPlugin() {};
	
	virtual bool Initialize(HINSTANCE hInst, RenderType type) = 0;
	virtual bool Shutdown() = 0;
	virtual void About(HWND hParent) = 0;
	virtual double Execute(double a, double b) = 0;
	virtual const std::string GetName() = 0;

	// 
	virtual void AddObject(GameObject* p_obj){}
	virtual void RemoveObject(GameObject* p_obj){}
	virtual bool AllUpdate() { return false; }
	virtual void AllStart() {}
	virtual void AllRelease() {}
};

typedef IPlugin* (*CREATEPLUGIN)(PluginManager &mgr);
// 플러그인 메인함수
extern "C" PLUGINDECL  IPlugin* CreatePlugin(PluginManager &mgr);


#endif  // IPLUGIN_H_
