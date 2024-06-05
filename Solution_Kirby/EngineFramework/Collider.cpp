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
	m_gameObj->SetPosition({ 
		m_body->GetPosition().x/* - (float)m_colOffset.x*/, 
		m_body->GetPosition().y /*- (float)m_colOffset.y*/,
		m_gameObj->Position().z});
}
const Vector2D Collider::GetColSize()
{
	return m_colSize;
}
void Collider::SetColSize(Vector2D size)
{
	m_colSize = size;

	MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
	b2BodyDef bodyDef;
	bodyDef.type = m_type;
	bodyDef.position.Set(m_gameObj->Position().x + (float)m_colOffset.x, m_gameObj->Position().y + (float)m_colOffset.y);
	m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
	if (!m_body) {
		cerr << "Failed to create body!" << endl;
		return;
	}
	b2PolygonShape box;
	box.SetAsBox(m_colSize.x * 0.8f, m_colSize.y * 0.8f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 0.01f;
	m_body->CreateFixture(&fixtureDef);


	m_body->GetUserData().pointer = (uintptr_t)this;
}
Vector2D& Collider::ColOffset()
{
	return m_colOffset;
}
void Collider::Initialize()
{

	//CollisionManager::GetInstance()->AddCollider(this);
	m_colSize.x = m_gameObj->Size().x;
	m_colSize.y = m_gameObj->Size().y;

	b2BodyDef bodyDef;
	bodyDef.type = m_type;
	bodyDef.position.Set(m_gameObj->Position().x + (float)m_colOffset.x, m_gameObj->Position().y + (float)m_colOffset.y);
	m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
	if (!m_body) {
		cerr << "Failed to create body!" << endl;
		return;
	}
	b2PolygonShape box; 
	box.SetAsBox(m_colSize.x * 0.8f, m_colSize.y * 0.8f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 0.2f;
	m_body->CreateFixture(&fixtureDef);


	m_body->GetUserData().pointer = (uintptr_t)this;

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

