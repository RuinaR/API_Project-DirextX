#include "pch.h"
#include "AttakObject.h"
#include "BoxCollider.h"
#include "game.h"
void AttakObject::CollisionEnter(Collider* other)
{
	m_gameObj->SetDestroy(true); //������
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
	m_texture = AnimationManager::LoadTexture(L"Bitmaps\\obj\\attack");
	m_gameObj->Size() = { 30,30 };
	m_gameObj->SetTag(TAG_ATTACK);
	m_gameObj->AddComponent(new BitmapRender(m_texture));
	m_gameObj->AddComponent(new BoxCollider(b2BodyType::b2_kinematicBody));
}

void AttakObject::Release()
{
	AnimationManager::ReleaseTexture(m_texture);
}

void AttakObject::Start()
{
}

void AttakObject::Update()
{
	m_gameObj->AddPosition({ m_speedX, 0.0f, 0.0f });
}

void AttakObject::SetSpeed(float speed)
{
	m_speedX = speed;
}
