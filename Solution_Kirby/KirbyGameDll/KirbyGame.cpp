#include "pch.h"
#include "KirbyGame.h"
#include "../include/PluginManager.h"

#include "StartScene.h"


KirbyGame::KirbyGame(PluginManager& p_mgr) :
	m_hManager(p_mgr),
	m_strName("KirbyGame")
{

}

bool KirbyGame::Initialize(HINSTANCE hInst, RenderType type)
{
	MainFrame::Create(hInst);
	MainFrame::GetInstance()->Initialize(TARGETFPS, new StartScene(), type);
	MainFrame::GetInstance()->Set();
	return true;
}

bool KirbyGame::Shutdown()
{
	return false;
}

void KirbyGame::About(HWND hParent)
{

}

double KirbyGame::Execute(double a, double b)
{
	return 0.0;
}

const std::string KirbyGame::GetName()
{
	return m_strName;
}

void KirbyGame::AddObject(GameObject* p_obj)
{

}

void KirbyGame::RemoveObject(GameObject* p_obj)
{
}

bool KirbyGame::AllUpdate()
{
	return MainFrame::GetInstance()->Update();
}

void KirbyGame::AllStart()
{

}

void KirbyGame::AllRelease()
{
	MainFrame::Destroy();
}


PLUGINDECL IPlugin* CreatePlugin(PluginManager& mgr)
{
	return new KirbyGame(mgr);
}