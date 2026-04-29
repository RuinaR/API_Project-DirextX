#include "pch.h"
#include "FBXRotateObj.h"
#include "SceneJsonUtility.h"

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

const char* FBXRotateObj::GetSerializableType() const
{
	return "FBXRotateObj";
}

std::string FBXRotateObj::Serialize() const
{
	return "{ }";
}

bool FBXRotateObj::Deserialize(const std::string& componentJson)
{
	return true;
}
