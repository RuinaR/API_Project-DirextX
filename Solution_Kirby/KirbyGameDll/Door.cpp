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
	m_gameObj->SetAngle(m_gameObj->GetAngle() + 0.001f);
}
