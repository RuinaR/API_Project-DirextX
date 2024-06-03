#include "pch.h"
#include "AttakObject.h"
#include "BoxCollider.h"
#include "game.h"
void AttakObject::CollisionEnter(Collider* other)
{
	m_gameObj->SetDestroy(true); //µ¥¹ÌÁö
}
void AttakObject::CollisionExit(Collider* other)
{
}
void AttakObject::Collision(Collider* other)
{
}
AttakObject::AttakObject()
	:Component()
{
}

AttakObject::~AttakObject()
{
}

void AttakObject::Initialize()
{
	m_bit = AnimationManager::LoadHBitmap("Bitmaps\\obj\\attack");
	m_gameObj->Size() = { 30,30 };
	m_gameObj->SetTag(TAG_ATTACK);
	m_gameObj->AddComponent(new BitmapRender(m_bit));
	m_gameObj->AddComponent(new BoxCollider());
}

void AttakObject::Release()
{
	AnimationManager::ReleaseHBitmap(m_bit);
}

void AttakObject::Start()
{
}

void AttakObject::Update()
{
	m_gameObj->AddPosition({ m_speedX, 0.0f });
}

void AttakObject::SetSpeed(float speed)
{
	m_speedX = speed;
}
