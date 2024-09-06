#include "pch.h"
#include "FBXRender.h"


FBXRender::FBXRender(std::string name)
	:m_name(name), m_tool()
{
}

void FBXRender::Initialize()
{
	m_tool.Initialize();
	m_tool.Load(m_name.c_str());
}

void FBXRender::Release()
{
	m_tool.Release();
}

void FBXRender::Start()
{
}

void FBXRender::Update()
{
	m_tool.ImguiUpdate();
}
