#include "pch.h"
#include "BoxCollider.h"


void BoxCollider::ColInit()
{
}

void BoxCollider::ColRelease()
{
}

//void BoxCollider::Update()
//{
//}

BoxCollider::BoxCollider(b2BodyType type) :Collider(type)
{
}

BoxCollider::~BoxCollider()
{
}

const char* BoxCollider::GetInspectorName() const
{
	return "Box Collider 2D";
}

const char* BoxCollider::GetSerializableType() const
{
	return "BoxCollider2D";
}
