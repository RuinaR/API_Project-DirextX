#include "pch.h"
#include "BoxCollider2D.h"


void BoxCollider2D::ColInit()
{
}

void BoxCollider2D::ColRelease()
{
}

b2Fixture* BoxCollider2D::CreateFixtureOnBody(b2Body* body)
{
	if (body == nullptr)
	{
		return nullptr;
	}

	b2PolygonShape box;
	if (m_ownsBody)
	{
		box.SetAsBox(m_colSize.x / 2.0f, m_colSize.y / 2.0f);
	}
	else
	{
		box.SetAsBox(
			m_colSize.x / 2.0f,
			m_colSize.y / 2.0f,
			b2Vec2(m_colOffset.x, m_colOffset.y),
			0.0f);
	}

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.8f;
	fixtureDef.restitution = 0.0f;
	fixtureDef.isSensor = m_isTrigger;
	return body->CreateFixture(&fixtureDef);
}

//void BoxCollider2D::Update()
//{
//}

BoxCollider2D::BoxCollider2D(b2BodyType type) :Collider2D(type)
{
}

BoxCollider2D::~BoxCollider2D()
{
}

const char* BoxCollider2D::GetInspectorName() const
{
	return "Box Collider 2D";
}

const char* BoxCollider2D::GetSerializableType() const
{
	return "BoxCollider2D";
}
