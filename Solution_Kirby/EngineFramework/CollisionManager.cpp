#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "Rigidbody.h"
CollisionManager* CollisionManager::m_Pthis = nullptr;


void CollisionManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new CollisionManager();
		m_Pthis->m_objVec = new vector<Collider*>();
	}
}

CollisionManager* CollisionManager::GetInstance()
{
	return m_Pthis;
}

void CollisionManager::Destroy()
{
	if (m_Pthis)
	{
		delete m_Pthis->m_objVec;
		m_Pthis->m_objVec = nullptr;
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void CollisionManager::AddCollider(Collider* col)
{
	m_objVec->push_back(col);
}

bool CollisionManager::UnregisterCollider(Collider* col)
{
	for (vector<Collider*>::iterator itr = m_objVec->begin(); itr != m_objVec->end(); itr++)
	{
		if ((*itr) == col)
		{
			m_objVec->erase(itr);
			return true;
		}
	}
	return false;
}

void CollisionManager::CollisionUpdate(vector<Collider*>* vec, const RECT& rect, int maxAreaCnt)
{
	//4�����ؼ� ��������� �浹ó��
	vector<Collider*>** rctDivision = new vector<Collider*>*[4];
	for (int i = 0; i < 4; i++)
	{
		rctDivision[i] = new vector<Collider*>();
	}
	int x = rect.left + (rect.right - rect.left) / 2;
	int y = rect.top + (rect.bottom - rect.top) / 2;

	if (DEBUGMODE)
	{
		HPEN newPen = CreatePen(PS_SOLID, 3, DEBUGCOLOR2);
		HPEN oldPen = (HPEN)SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), newPen);

		MoveToEx(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), x - Camera::GetInstance()->GetPos().x, rect.top - Camera::GetInstance()->GetPos().y, NULL);
		LineTo(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), x - Camera::GetInstance()->GetPos().x, rect.bottom - Camera::GetInstance()->GetPos().y);
		MoveToEx(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), rect.left - Camera::GetInstance()->GetPos().x, y - Camera::GetInstance()->GetPos().y, NULL);
		LineTo(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), rect.right - Camera::GetInstance()->GetPos().x, y - Camera::GetInstance()->GetPos().y);

		SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), oldPen);
		DeleteObject(newPen);
	}
	for (vector<Collider*>::iterator itr = vec->begin(); itr != vec->end(); itr++)
	{
		RECT tmp;
		RECT dst =
		{
		(*itr)->GetGameObject()->Position().x + (*itr)->ColOffset().x ,
		(*itr)->GetGameObject()->Position().y + (*itr)->ColOffset().y ,
		(*itr)->GetGameObject()->Position().x + (*itr)->ColOffset().x + (*itr)->ColSize().x,
		(*itr)->GetGameObject()->Position().y + (*itr)->ColOffset().y + (*itr)->ColSize().y
		};
		//0�� ����
		RECT area0 = { rect.left , rect.top, x, y };
		//1�� ����
		RECT area1 = { x, rect.top, rect.right, y };
		//2�� ����
		RECT area2 = { rect.left, y, x, rect.bottom};
		//3�� ����
		RECT area3 = { x, y, rect.right, rect.bottom };

		if (IntersectRect(&tmp, &area0, &dst))
			rctDivision[0]->push_back((*itr));
		if (IntersectRect(&tmp, &area1, &dst))
			rctDivision[1]->push_back((*itr));
		if (IntersectRect(&tmp, &area2, &dst))
			rctDivision[2]->push_back((*itr));
		if (IntersectRect(&tmp, &area3, &dst))
			rctDivision[3]->push_back((*itr));
	}

	RECT r1, r2, tmpr;
	set<Collider*> cloneColSet;
	for (int i = 0; i < 4; i++)
	{
		if (rctDivision[i]->size() >= maxAreaCnt)
		{
			RECT area;
			switch (i)
			{
			case 0:
				area.left = rect.left;
				area.right = rect.left + (rect.right - rect.left) / 2;
				area.top = rect.top;
				area.bottom = rect.top + (rect.bottom - rect.top) / 2;
				break;
			case 1:
				area.left = rect.left + (rect.right - rect.left) / 2;
				area.right = rect.right;
				area.top = rect.top;
				area.bottom = rect.top + (rect.bottom - rect.top) / 2;
				break;
			case 2:
				area.left = rect.left;
				area.right = rect.left + (rect.right - rect.left) / 2;
				area.top = rect.top + (rect.bottom - rect.top) / 2;
				area.bottom =rect.bottom;
				break;
			case 3:
				area.left = rect.left + (rect.right - rect.left) / 2;
				area.right = rect.right;
				area.top = rect.top + (rect.bottom - rect.top) / 2;
				area.bottom = rect.bottom;
				break;
			}
			CollisionUpdate(rctDivision[i], area, maxAreaCnt);
			continue;
		}
		for (vector<Collider*>::iterator itr1 = rctDivision[i]->begin(); itr1 != rctDivision[i]->end(); itr1++)
		{
			if (DEBUGMODE)
			{
				TextOut(WindowFrame::GetInstance()->GetBuffer()->GetHDC(),
					(*itr1)->GetGameObject()->Position().x - Camera::GetInstance()->GetPos().x + 10,
					(*itr1)->GetGameObject()->Position().y - Camera::GetInstance()->GetPos().y + 10,
					TEXT("��"), 1);
			}

			cloneColSet = set<Collider*>((*itr1)->SetCol()->begin(), (*itr1)->SetCol()->end());
			(*itr1)->SetCol()->clear();

			r1 = { (long)(*itr1)->ColOffset().x + (long)(*itr1)->GetGameObject()->Position().x,
						(long)(*itr1)->ColOffset().y + (long)(*itr1)->GetGameObject()->Position().y,
						(long)(*itr1)->ColOffset().x + (long)(*itr1)->GetGameObject()->Position().x + (long)(*itr1)->ColSize().x,
						(long)(*itr1)->ColOffset().y + (long)(*itr1)->GetGameObject()->Position().y + (long)(*itr1)->ColSize().y };
			vector<Collider*>::iterator tmpItr = itr1;
			for (vector<Collider*>::iterator itr2 = ++tmpItr; itr2 != rctDivision[i]->end(); itr2++)
			{
				if ((*itr1) == (*itr2))
					continue;
				if (!(*itr1)->GetGameObject()->GetActive() || !(*itr2)->GetGameObject()->GetActive())
					continue;
				r2 = { (long)(*itr2)->ColOffset().x + (long)(*itr2)->GetGameObject()->Position().x,
							(long)(*itr2)->ColOffset().y + (long)(*itr2)->GetGameObject()->Position().y,
							(long)(*itr2)->ColOffset().x + (long)(*itr2)->GetGameObject()->Position().x + (long)(*itr2)->ColSize().x,
							(long)(*itr2)->ColOffset().y + (long)(*itr2)->GetGameObject()->Position().y + (long)(*itr2)->ColSize().y };
				if (IntersectRect(&tmpr, &r1, &r2)) //�浹
				{
					(*itr1)->SetCol()->insert(*itr2);
					bool isfind = false;
					if (cloneColSet.find(*itr2) != cloneColSet.end())
						isfind = true;
					for (vector<Component*>::iterator citr = (*itr1)->GetGameObject()->GetComponentVec()->begin();
						citr != (*itr1)->GetGameObject()->GetComponentVec()->end();
						citr++)
					{
						if (!isfind) (*citr)->OnCollisionEnter((*itr2));
						else (*citr)->OnCollision((*itr2));
					}
					for (vector<Component*>::iterator citr = (*itr2)->GetGameObject()->GetComponentVec()->begin();
						citr != (*itr2)->GetGameObject()->GetComponentVec()->end();
						citr++)
					{
						if (!isfind) (*citr)->OnCollisionEnter((*itr1));
						else (*citr)->OnCollision((*itr1));
					}
				}
			}

			for (set<Collider*>::iterator itr3 = cloneColSet.begin(); itr3 != cloneColSet.end(); itr3++)
			{
				if ((*itr1)->SetCol()->find(*itr3) == (*itr1)->SetCol()->end()) //������ �浹������ ���� �浹 ���°� �ƴ϶��, �Ǵ� �浹�ߴ� ��ü�� ��ȿ�� ���°� �ƴ϶��
				{
					for (vector<Component*>::iterator citr = (*itr1)->GetGameObject()->GetComponentVec()->begin();
						citr != (*itr1)->GetGameObject()->GetComponentVec()->end();
						citr++)
					{
						(*citr)->OnCollisionExit((*itr3));
					}
				}
			}
		}
	}


	for (int i = 0; i < 4; i++)
	{
		delete rctDivision[i];
	}
	delete[] rctDivision;
}

void CollisionManager::Update()
{
	RECT rect;
	//GetClientRect(WindowFrame::GetInstance()->GetHWND(), &rect);
	//rect.left += Camera::GetInstance()->GetPos().x;
	//rect.right += Camera::GetInstance()->GetPos().x;
	//rect.top += Camera::GetInstance()->GetPos().y;
	//rect.bottom += Camera::GetInstance()->GetPos().y;

	rect.left = 0;
	rect.right = 10000;
	rect.top = 0;
	rect.bottom = 10000;

	CollisionUpdate(m_objVec, rect, 5);
}

void CollisionManager::Clear()
{
	m_objVec->clear();
}

int CollisionManager::Count()
{
	return m_objVec->size();
}
