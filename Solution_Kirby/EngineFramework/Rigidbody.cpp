#include "pch.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "Component.h"
#include "DebugWindow.h"

BOOL Rigidbody::SetNoIntersect(const LPRECT pHold, LPRECT pRect)
{
	RECT rcInter = { 0 };
	if (IntersectRect(&rcInter, pHold, pRect))
	{
		int nW = rcInter.right - rcInter.left;
		int nH = rcInter.bottom - rcInter.top;
		if (nW > nH)
		{
			if (rcInter.top == pHold->top)
			{
				m_isOnLand = true;
				pRect->top -= nH;
				pRect->bottom -= nH;
				if (m_velocity.y > 0)
					m_velocity.y = 0;
			}
			else if (rcInter.bottom == pHold->bottom)
			{
				pRect->top += nH;
				pRect->bottom += nH;
				if (m_velocity.y < 0)
					m_velocity.y = 0;
			}
		}
		else
		{
			if (rcInter.left == pHold->left)
			{
				pRect->left -= nW;
				pRect->right -= nW;
				if (m_velocity.x > 0)
					m_velocity.x = 0;
			}
			else if (rcInter.right == pHold->right)
			{
				pRect->left += nW;
				pRect->right += nW;
				if (m_velocity.x < 0)
					m_velocity.x = 0;
			}
		}
		return TRUE;
	}
	return FALSE;

}

void Rigidbody::Initialize()
{
	m_velocity = { 0.0f, 0.0f };
}

void Rigidbody::Release()
{
}

void Rigidbody::Start()
{
	m_box = m_gameObj->GetComponent<BoxCollider>();
}

void Rigidbody::Update()
{
	if (!m_isOnLand)
	{
		m_velocity.y += m_gravity * MainFrame::GetInstance()->DeltaTime();
	}
	else
	{
		if (m_velocity.y > 0)
			m_velocity.y = 0.0f;
	}
	if (!m_isNoFriction)
	{
		m_velocity.x *= 1.0f - m_friction * MainFrame::GetInstance()->DeltaTime();
		m_velocity.y *= 1.0f - m_friction * MainFrame::GetInstance()->DeltaTime();
	}
	if (abs(m_velocity.x) < 20.0f && abs(m_velocity.y) < 20.0f)
		return;

	Vector2D move;
	move.x = (float)(m_velocity.x * MainFrame::GetInstance()->DeltaTime());
	move.y = (float)(m_velocity.y * MainFrame::GetInstance()->DeltaTime());

	m_gameObj->AddPosition(move);
}

void Rigidbody::CollisionEnter(Collider* other)
{
	//int d = 10;

	//if (m_box->GetTrigger() || other->GetTrigger())
	//	return;

	//if (m_gameObj->Position().y + m_box->ColSize().y + m_box->ColOffset().y <
	//	other->GetGameObject()->Position().y + other->ColOffset().y + d)
	//{
	//	m_isOnLand = true;
	//}
}

void Rigidbody::CollisionExit(Collider* other)
{
	int d = 1;
	for (set<Collider*>::iterator itr = m_box->SetCol()->begin(); itr != m_box->SetCol()->end(); itr++)
	{
		if (!(*itr)->GetTrigger() &&
			!m_box->GetTrigger() && 
			(m_gameObj->Position().y + m_box->ColSize().y + m_box->ColOffset().y<
			(*itr)->GetGameObject()->Position().y + (*itr)->ColOffset().y + d))
			return;
	}
	m_isOnLand = false;
}

void Rigidbody::Collision(Collider* other)
{
	if (m_box->GetTrigger() || other->GetTrigger())
		return;

	RECT r1, r2;
	long d = 2;
	r1 = { (long)m_box->ColOffset().x + (long)m_gameObj->Position().x,
			(long)m_box->ColOffset().y + (long)m_gameObj->Position().y,
			(long)m_box->ColOffset().x + (long)(m_gameObj->Position().x + m_box->ColSize().x),
			(long)m_box->ColOffset().y + (long)(m_gameObj->Position().y + m_box->ColSize().y) };
	r2 = { (long)other->ColOffset().x + (long)other->GetGameObject()->Position().x + d,
			(long)other->ColOffset().y + (long)other->GetGameObject()->Position().y + d,
			(long)other->ColOffset().x + (long)(other->GetGameObject()->Position().x + other->ColSize().x - d),
			(long)other->ColOffset().y + (long)(other->GetGameObject()->Position().y + other->ColSize().y - d) };

	SetNoIntersect(&r2, &r1);
	m_gameObj->SetPosition(Vector2D({ (float)r1.left - m_box->ColOffset().x, (float)r1.top - m_box->ColOffset().y }));

	return;
}

float Rigidbody::GetGravity()
{
	return m_gravity;
}

void Rigidbody::SetGravity(float g)
{
	m_gravity = g;
}

void Rigidbody::SetFriction(float f)
{
	m_friction = f;
}

void Rigidbody::AddForce(Vector2D vec)
{
	m_velocity.x += vec.x;
	m_velocity.y += vec.y;
}

bool Rigidbody::GetIsOnLand()
{
	//cout << m_isOnLand << endl;
	return m_isOnLand;
}

Vector2D& Rigidbody::Velocity()
{
	return m_velocity;
}

void Rigidbody::SetNoFriction(bool nf)
{
	m_isNoFriction = nf;
}
