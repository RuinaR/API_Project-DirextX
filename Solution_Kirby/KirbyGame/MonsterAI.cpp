#include "pch.h"
#include "MonsterAI.h"

void MonsterAI::CollisionEnter(Collider* other)
{

}

void MonsterAI::CollisionExit(Collider* other)
{
}

void MonsterAI::Collision(Collider* other)
{
	if (abs(m_bo->GetBody()->GetLinearVelocity().x) < 10)
	{
		if (m_arrow == Arrow::left)
		{
			m_arrowVal = 1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::right]);
			m_arrow = Arrow::right;
		}
		else
		{
			m_arrowVal = -1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::left]);
			m_arrow = Arrow::left;
		}
		m_bo->GetBody()->SetLinearVelocity({ m_arrowVal * m_speed ,m_bo->GetBody()->GetLinearVelocity().y + 50 });
	}
}

void MonsterAI::Initialize()
{
	m_arrowVal = 1;

	m_ar = m_gameObj->GetComponent<AnimationRender>();
	if (m_ar == nullptr)
	{
		m_ar = new AnimationRender(m_anim[(int)Arrow::right]);
		m_gameObj->AddComponent(m_ar);
	}

	m_bo = m_gameObj->GetComponent<BoxCollider>();
	if (m_bo == nullptr)
	{
		m_bo = new BoxCollider(b2BodyType::b2_dynamicBody);
		m_gameObj->AddComponent(m_bo);
	}
	m_bo->CreateBody(
		{ 0 ,0 },
		{ m_gameObj->Size().x / 2 ,m_gameObj->Size().y / 2 });
	m_bo->GetBody()->SetLinearVelocity({ m_arrowVal * m_speed ,m_bo->GetBody()->GetLinearVelocity().y});
}

void MonsterAI::Release()
{
}

void MonsterAI::Start()
{
	
}

void MonsterAI::Update()
{
}


MonsterAI::MonsterAI(Animation left, Animation right)
{
	m_anim[(int)Arrow::left] = left;
	m_anim[(int)Arrow::right] = right;
}  

MonsterAI::~MonsterAI()
{
}
