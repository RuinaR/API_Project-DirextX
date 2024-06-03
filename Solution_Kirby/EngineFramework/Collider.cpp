#include "pch.h"
#include "Collider.h"
set<Collider*>* Collider::SetCol()
{
	return m_ColObjSet;
}
void Collider::Start()
{
}
void Collider::Update()
{
}
Vector2D& Collider::ColSize()
{
	return m_colSize;
}
Vector2D& Collider::ColOffset()
{
	return m_colOffset;
}
void Collider::Initialize()
{
	ColInit();
	m_ColObjSet = new set<Collider*>();
	CollisionManager::GetInstance()->AddCollider(this);
	m_colSize.x *= COLSIZEOFFSET;
	m_colSize.y *= COLSIZEOFFSET;
}

void Collider::Release()
{
	m_ColObjSet->clear();
	delete m_ColObjSet;
	CollisionManager::GetInstance()->UnregisterCollider(this);
	ColRelease();
}

void Collider::SetTrigger(bool b)
{
	m_isTrigger = b;
}

bool Collider::GetTrigger()
{
	return m_isTrigger;
}

