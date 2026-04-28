#include "pch.h"
#include "FBXRotateObj.h"

void FBXRotateObj::Initialize()
{
}

void FBXRotateObj::Release()
{
}

void FBXRotateObj::Start()
{
}

void FBXRotateObj::Update()
{
	if (MainFrame::GetInstance()->GetRenderType() == RenderType::Edit)
		return;

	m_gameObj->SetAngleY(m_gameObj->GetAngleY() + 1.0f);
}
