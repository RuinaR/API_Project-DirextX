#pragma once
#include "../include/plugin.h"
class KirbyGame : public IPlugin
{
public:
	KirbyGame(PluginManager& p_mgr);

	virtual bool Initialize(HINSTANCE hInst, RenderType type);
	virtual bool Shutdown();
	virtual void About(HWND hParent);
	virtual double Execute(double a, double b);
	virtual const std::string GetName();


	void AddObject(GameObject* p_obj) override;
	void RemoveObject(GameObject* p_obj) override;
	bool AllUpdate() override;
	void AllStart() override;
	void AllRelease() override;

private:
	PluginManager& m_hManager;
	std::string m_strName;
};

