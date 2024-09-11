#include "pch.h"
#include "Door.h"

void Door::Initialize()
{
}

void Door::Release()
{
}

void Door::Start()
{
}

void Door::Update()
{
	m_gameObj->SetAngleZ(m_gameObj->GetAngleZ() + 0.001f);
}
