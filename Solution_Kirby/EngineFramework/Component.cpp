#include "pch.h"
#include "Component.h"
Component::Component()
{
}

Component::~Component()
{
}

void Component::InitGameObj(GameObject* obj)
{
	m_gameObj = obj;
}

GameObject* Component::GetGameObject()
{
	return this->m_gameObj;
}

void Component::OnLBtnDown()
{

}

void Component::OnLBtnUp()
{

}

void Component::OnRBtnDown()
{
	
}

void Component::OnRBtnUp()
{
	
}

void Component::OnMouseHover()
{
	
}
