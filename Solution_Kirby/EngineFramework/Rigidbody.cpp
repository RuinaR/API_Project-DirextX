#include "pch.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "Component.h"
#include "DebugWindow.h"

BOOL IntersectRectWithOutput(RECT* rcInter, const RECT* rc1, const RECT* rc2)
{
	LONG left1 = min(rc1->left, rc1->right);
	LONG right1 = max(rc1->left, rc1->right);
	LONG top1 = max(rc1->top, rc1->bottom);
	LONG bottom1 = min(rc1->top, rc1->bottom);

	LONG left2 = min(rc2->left, rc2->right);
	LONG right2 = max(rc2->left, rc2->right);
	LONG top2 = max(rc2->top, rc2->bottom);
	LONG bottom2 = min(rc2->top, rc2->bottom);

	LONG left = max(left1, left2);
	LONG right = min(right1, right2);
	LONG top = min(top1, top2);
	LONG bottom = max(bottom1, bottom2);

	if (left < right && top > bottom)
	{
		rcInter->left = left;
		rcInter->top = top;
		rcInter->right = right;
		rcInter->bottom = bottom;

		return TRUE;
	}

	return FALSE;
}

BOOL Rigidbody::SetNoIntersect(const LPRECT pHold, LPRECT pRect)
{
	RECT rcInter = { 0 };
	if (IntersectRectWithOutput(&rcInter, pHold, pRect))
	{
		int nW = rcInter.right - rcInter.left;
		int nH = rcInter.top - rcInter.bottom;
		if (nW > nH)
		{
			if (rcInter.top == pHold->top)
			{
				m_isOnLand = true;
				pRect->top += nH; // y 좌표를 위로 이동
				pRect->bottom += nH; // y 좌표를 위로 이동
				if (m_velocity.y < 0)
					m_velocity.y = 0;
			}
			else if (rcInter.bottom == pHold->bottom)
			{
				pRect->top -= nH; // y 좌표를 아래로 이동
				pRect->bottom -= nH; // y 좌표를 아래로 이동
				if (m_velocity.y > 0)
					m_velocity.y = 0;
			}
		}
		else
		{
			if (rcInter.left == pHold->left)
			{
				pRect->left += nW; // x 좌표를 오른쪽으로 이동
				pRect->right += nW; // x 좌표를 오른쪽으로 이동
				if (m_velocity.x > 0)
					m_velocity.x = 0;
			}
			else if (rcInter.right == pHold->right)
			{
				pRect->left -= nW; // x 좌표를 왼쪽으로 이동
				pRect->right -= nW; // x 좌표를 왼쪽으로 이동
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
		m_velocity.y -= m_gravity * MainFrame::GetInstance()->DeltaTime();
	}
	else
	{
		if (m_velocity.y < 0)
			m_velocity.y = 0.0f;
	}
	if (!m_isNoFriction)
	{
		m_velocity.x *= 1.0f - m_friction * MainFrame::GetInstance()->DeltaTime();
		m_velocity.y *= 1.0f - m_friction * MainFrame::GetInstance()->DeltaTime();
	}
	if (abs(m_velocity.x) < 20.0f && abs(m_velocity.y) < 20.0f)
		return;

	D3DXVECTOR3 move;
	move.x = (float)(m_velocity.x * MainFrame::GetInstance()->DeltaTime());
	move.y = (float)(m_velocity.y * MainFrame::GetInstance()->DeltaTime());
	move.z = 0.0f;
	m_gameObj->AddPosition(move);
}

void Rigidbody::CollisionEnter(Collider* other)
{

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
	long d = 0;
	r1 = { (long)m_box->ColOffset().x + (long)m_gameObj->Position().x,
			(long)m_box->ColOffset().y + (long)(m_gameObj->Position().y),
			(long)m_box->ColOffset().x + (long)(m_gameObj->Position().x + m_box->ColSize().x),
			(long)m_box->ColOffset().y + (long)(m_gameObj->Position().y - m_box->ColSize().y) };
	r2 = { (long)other->ColOffset().x + (long)other->GetGameObject()->Position().x - d,
			(long)other->ColOffset().y + (long)(other->GetGameObject()->Position().y + d),
			(long)other->ColOffset().x + (long)(other->GetGameObject()->Position().x + other->ColSize().x + d),
			(long)other->ColOffset().y + (long)(other->GetGameObject()->Position().y - other->ColSize().y - d) };

	if (SetNoIntersect(&r2, &r1))
		m_gameObj->SetPosition(D3DXVECTOR3({ (float)(r1.left - m_box->ColOffset().x), (float)(r1.top - m_box->ColOffset().y), 
			m_gameObj->Position().z}));

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
