#include "pch.h"
#include "MonsterAI.h"

void MonsterAI::CollisionEnter(Collider* other)
{
	if (other->GetGameObject()->GetTag() == TAG_MONSTER ||
		other->GetGameObject()->GetTag() == TAG_LAND)
	{
		//방향 전환
		if (m_arrow == Arrow::left)
		{
			m_arrowVal = 1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::right]);
			m_arrow = Arrow::right;
			m_checker->GetCollider()->ColOffset().x = m_bo->ColOffset().x + m_gameObj->Size().x / 2;
		}
		else
		{
			m_arrowVal = -1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::left]);
			m_arrow = Arrow::left;
			m_checker->GetCollider()->ColOffset().x = m_bo->ColOffset().x - m_gameObj->Size().x / 8;
		}
		m_gameObj->AddPosition({ (float)(m_arrowVal * m_speed * MainFrame::GetInstance()->DeltaTime()) ,0,0 });
	}
}

void MonsterAI::CollisionExit(Collider* other)
{
}

void MonsterAI::Collision(Collider* other)
{
}

void MonsterAI::Initialize()
{
	string path[(int)Arrow::max];
	path[(int)Arrow::left] = "Bitmaps\\monster\\" + m_namePath + "\\left";
	path[(int)Arrow::right] = "Bitmaps\\monster\\" + m_namePath + "\\right";

	m_anim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
	m_anim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);

	GameObject* newGo = new GameObject();
	m_checker = new LandChecker();
	newGo->SetOrderInLayer(3);
	newGo->SetPosition(m_gameObj->Position());
	newGo->Size() = {0.0f, 0.0f};
	newGo->AddComponent(m_checker);
	newGo->SetParent(m_gameObj);
	newGo->InitializeSet();

	m_ar = m_gameObj->GetComponent<AnimationRender>();
	if (m_ar == nullptr)
	{
		m_ar = new AnimationRender(m_anim[(int)Arrow::right]);
		m_gameObj->AddComponent(m_ar);
	}

	m_bo = m_gameObj->GetComponent<BoxCollider>();
	if (m_bo == nullptr)
	{
		m_bo = new BoxCollider();
		m_gameObj->AddComponent(m_bo);
	}
	m_bo->ColSize().x = m_gameObj->Size().x / 2;
	m_bo->ColSize().y = m_gameObj->Size().y / 2;
	m_bo->ColOffset().x = m_gameObj->Size().x / 4;
	m_bo->ColOffset().y = m_gameObj->Size().y / 4;
}

void MonsterAI::Release()
{
}

void MonsterAI::Start()
{
	int m_arrowVal = 1;
	m_checker->GetCollider()->ColSize().x = m_bo->ColSize().x / 4;
	m_checker->GetCollider()->ColSize().y = m_bo->ColSize().y / 4;
	m_checker->GetCollider()->ColOffset().x = m_bo->ColOffset().x + m_gameObj->Size().x / 2;
	m_checker->GetCollider()->ColOffset().y = m_bo->ColOffset().y - m_gameObj->Size().y / 2;
}

void MonsterAI::Update()
{
	if (m_checker == nullptr)
		return;


	if (!m_checker->IsOnLand())
	{
		//방향 전환
		if (m_arrow == Arrow::left)
		{
			m_arrowVal = 1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::right]);
			m_arrow = Arrow::right;
			m_checker->GetCollider()->ColOffset().x = m_bo->ColOffset().x + m_gameObj->Size().x / 2;
		}
		else
		{
			m_arrowVal = -1;
			m_ar->ChangeAnim(m_anim[(int)Arrow::left]);
			m_arrow = Arrow::left;
			m_checker->GetCollider()->ColOffset().x = m_bo->ColOffset().x - m_gameObj->Size().x / 8;
		}
	}
	m_gameObj->AddPosition({ (float)(m_arrowVal * m_speed * MainFrame::GetInstance()->DeltaTime()) ,0,0 });
}


MonsterAI::MonsterAI(string namePath)
{
	m_namePath = namePath;
}

MonsterAI::~MonsterAI()
{
}
