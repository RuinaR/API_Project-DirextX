#include "pch.h"
#include "Collider.h"

Collider::Collider(b2BodyType type)
	:m_type(type)
{
}
void Collider::Start()
{
}
void Collider::Update()
{
	if (m_body->GetType() != b2_staticBody)
	{
		m_gameObj->SetPosition({
			m_body->GetPosition().x/* - (float)m_colOffset.x*/,
			m_body->GetPosition().y /*- (float)m_colOffset.y*/,
			m_gameObj->Position().z });
	}
}
const Vector2D& Collider::GetColSize()
{
	return m_colSize;
}

const Vector2D& Collider::GetColOffset()
{
	return m_colOffset;
}

void Collider::CreateBody(Vector2D offset, Vector2D size)
{
	m_colSize = size;
	m_colOffset = offset;

	b2BodyDef bodyDef;
	bodyDef.type = m_type;
	bodyDef.fixedRotation = true;
	bodyDef.position.Set(
		m_gameObj->Position().x + (float)m_colOffset.x,
		m_gameObj->Position().y + (float)m_colOffset.y);
	if (!MainFrame::GetInstance()->GetBox2dWorld()->IsLocked())
	{
		m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
	}
	else 
	{
		cerr << "Failed to create body!" << endl;
		return;
	}
	b2PolygonShape box;
	box.SetAsBox(m_colSize.x / 2.0f, m_colSize.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 0.0f;
	m_body->CreateFixture(&fixtureDef);

	m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;
}
void Collider::Initialize()
{
	//CollisionManager::GetInstance()->AddCollider(this);
	ColInit();
}

void Collider::Release()
{
	//CollisionManager::GetInstance()->UnregisterCollider(this);
	MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
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

b2Body* Collider::GetBody()
{
	return m_body;
}

