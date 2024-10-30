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
	m_gameObj->SetAngleY(m_gameObj->GetAngleY() + 1.0f);
}
