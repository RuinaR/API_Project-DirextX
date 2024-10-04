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
	m_texture = AnimationManager::LoadTexture("Bitmaps\\obj\\attack.bmp");
	m_gameObj->Size3D() = { 30,30,1 };
	m_gameObj->SetTag(TAG_ATTACK);
	m_gameObj->AddComponent(new ImageRender(m_texture));
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
