#include "pch.h"
#include "BoxCollider.h"


void BoxCollider::ColInit()
{
	if (m_gameObj == nullptr)
		return;
	m_colSize.x = m_gameObj->Size().x;
	m_colSize.y = m_gameObj->Size().y;
}

void BoxCollider::ColRelease()
{
}

BoxCollider::BoxCollider() :Collider()
{
}

BoxCollider::~BoxCollider()
{
}