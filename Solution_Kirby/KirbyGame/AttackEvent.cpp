#include "pch.h"
#include "AttackEvent.h"
#include "Collider.h"
#include "game.h"

void AttackEvent::CollisionEnter(Collider* other)
{
	if (other->GetGameObject()->GetTag() == TAG_MONSTER)
	{
		other->GetGameObject()->SetDestroy(true);  //데미지 처리 할 것
		m_gameObj->SetDestroy(true);
	}
}

void AttackEvent::CollisionExit(Collider* other)
{
}

void AttackEvent::Collision(Collider* other)
{
}

void AttackEvent::Initialize()
{
	m_timer.resetTotalDeltaTime();
	m_timer.tick();
}

void AttackEvent::Release()
{
}

void AttackEvent::Start()
{
}

void AttackEvent::Update()
{
	m_timer.tick();
	if (m_timer.getTotalDeltaTime() > 0.5f)
	{
		m_gameObj->SetDestroy(true);
	}
}

void AttackEvent::SetDmg(int dmg)
{
	m_dmg = dmg;
}
