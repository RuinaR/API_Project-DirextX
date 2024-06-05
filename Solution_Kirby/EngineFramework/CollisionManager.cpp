#include "pch.h"
//#include "GameObject.h"
//#include "Component.h"
//#include "Collider.h"
//#include "BoxCollider.h"
//#include "Rigidbody.h"
//CollisionManager* CollisionManager::m_Pthis = nullptr;
//
//BOOL IsBoxIntersect(Collider* b1, Collider* b2)
//{
//	float b1_left = b1->GetGameObject()->Position().x + b1->ColOffset().x;
//	float b1_right = b1_left + b1->ColSize().x;
//	float b1_bottom = b1->GetGameObject()->Position().y + b1->ColOffset().y;
//	float b1_top = b1_bottom + b1->ColSize().y; // 위로 갈수록 y 좌표가 증가하는 좌표계
//	
//
//	float b2_left = b2->GetGameObject()->Position().x + b2->ColOffset().x;
//	float b2_right = b2_left + b2->ColSize().x;
//	float b2_bottom = b2->GetGameObject()->Position().y + b2->ColOffset().y;
//	float b2_top = b2_bottom + b2->ColSize().y; // 위로 갈수록 y 좌표가 증가하는 좌표계
//	
//
//	// 충돌 검사
//	if (b1_right < b2_left || b1_left > b2_right || b1_bottom > b2_top || b1_top < b2_bottom)
//	{
//		return false;
//	}
//	return true;
//}
//
//BOOL IsBoxIntersect(RECT* b1, RECT* b2)
//{
//	// 충돌 검사
//	if (b1->right < b2->left || b1->left > b2->right || b1->bottom > b2->top|| b1->top < b2->bottom)
//	{
//		return false;
//	}
//	return true;
//}
//
//void CollisionManager::Create()
//{
//	if (!m_Pthis)
//	{
//		m_Pthis = new CollisionManager();
//		m_Pthis->m_objVec = new vector<Collider*>();
//	}
//}
//
//CollisionManager* CollisionManager::GetInstance()
//{
//	return m_Pthis;
//}
//
//void CollisionManager::Destroy()
//{
//	if (m_Pthis)
//	{
//		delete m_Pthis->m_objVec;
//		m_Pthis->m_objVec = nullptr;
//		delete m_Pthis;
//		m_Pthis = nullptr;
//	}
//}
//
//void CollisionManager::AddCollider(Collider* col)
//{
//	m_objVec->push_back(col);
//}
//
//bool CollisionManager::UnregisterCollider(Collider* col)
//{
//	for (vector<Collider*>::iterator itr = m_objVec->begin(); itr != m_objVec->end(); itr++)
//	{
//		if ((*itr) == col)
//		{
//			m_objVec->erase(itr);
//			return true;
//		}
//	}
//	return false;
//}
//
//void CollisionManager::CollisionUpdate(vector<Collider*>* vec, const RECT& rect, int maxAreaCnt)
//{
//	//4분할해서 재귀적으로 충돌처리
//	vector<Collider*>** rctDivision = new vector<Collider*>*[4];
//	for (int i = 0; i < 4; i++)
//	{
//		rctDivision[i] = new vector<Collider*>();
//	}
//	int x = rect.left + (rect.right - rect.left) / 2;
//	int y = rect.bottom + (rect.top - rect.bottom) / 2;
//
//	RECT areas[4];
//	//0번 영역
//	areas[0] = {rect.left , rect.top, x, y};
//	//1번 영역
//	areas[1] = {x, rect.top, rect.right, y};
//	//2번 영역
//	areas[2] = {rect.left, y, x, rect.bottom};
//	//3번 영역
//	areas[3] = {x, y, rect.right, rect.bottom};
//	for (vector<Collider*>::iterator itr = vec->begin(); itr != vec->end(); itr++)
//	{
//		RECT dst =
//		{
//		(*itr)->GetGameObject()->Position().x + (*itr)->ColOffset().x ,
//		(*itr)->GetGameObject()->Position().y - (*itr)->ColOffset().y,
//		(*itr)->GetGameObject()->Position().x + (*itr)->ColOffset().x + (*itr)->ColSize().x,
//		(*itr)->GetGameObject()->Position().y + (*itr)->ColOffset().y - (*itr)->ColSize().y
//		};
//
//		if (IsBoxIntersect(&areas[0], &dst))
//			rctDivision[0]->push_back((*itr));
//		if (IsBoxIntersect(&areas[1], &dst))
//			rctDivision[1]->push_back((*itr));
//		if (IsBoxIntersect(&areas[2], &dst))
//			rctDivision[2]->push_back((*itr));
//		if (IsBoxIntersect(&areas[3], &dst))
//			rctDivision[3]->push_back((*itr));
//	}
//
//	set<Collider*> cloneColSet;
//	for (int i = 0; i < 4; i++)
//	{
//		if (rctDivision[i]->size() >= maxAreaCnt)
//		{
//			CollisionUpdate(rctDivision[i], areas[i], maxAreaCnt);
//			continue;
//		}
//		for (vector<Collider*>::iterator itr1 = rctDivision[i]->begin(); itr1 != rctDivision[i]->end(); itr1++)
//		{
//			cloneColSet = set<Collider*>((*itr1)->SetCol()->begin(), (*itr1)->SetCol()->end());
//			(*itr1)->SetCol()->clear();
//
//			
//			vector<Collider*>::iterator tmpItr = itr1;
//			for (vector<Collider*>::iterator itr2 = ++tmpItr; itr2 != rctDivision[i]->end(); itr2++)
//			{
//				if ((*itr1) == (*itr2))
//					continue;
//				if (!(*itr1)->GetGameObject()->GetActive() || !(*itr2)->GetGameObject()->GetActive())
//					continue;
//				
//				if (IsBoxIntersect(*itr1, *itr2)) //충돌
//				{
//					(*itr1)->SetCol()->insert(*itr2);
//					bool isfind = false;
//					if (cloneColSet.find(*itr2) != cloneColSet.end())
//						isfind = true;
//					for (vector<Component*>::iterator citr = (*itr1)->GetGameObject()->GetComponentVec()->begin();
//						citr != (*itr1)->GetGameObject()->GetComponentVec()->end();
//						citr++)
//					{
//						if (!isfind) (*citr)->OnCollisionEnter((*itr2));
//						else (*citr)->OnCollision((*itr2));
//					}
//					for (vector<Component*>::iterator citr = (*itr2)->GetGameObject()->GetComponentVec()->begin();
//						citr != (*itr2)->GetGameObject()->GetComponentVec()->end();
//						citr++)
//					{
//						if (!isfind) (*citr)->OnCollisionEnter((*itr1));
//						else (*citr)->OnCollision((*itr1));
//					}
//				}
//			}
//
//			for (set<Collider*>::iterator itr3 = cloneColSet.begin(); itr3 != cloneColSet.end(); itr3++)
//			{
//				if ((*itr1)->SetCol()->find(*itr3) == (*itr1)->SetCol()->end()) //이전에 충돌했지만 현재 충돌 상태가 아니라면, 또는 충돌했던 객체가 유효한 상태가 아니라면
//				{
//					for (vector<Component*>::iterator citr = (*itr1)->GetGameObject()->GetComponentVec()->begin();
//						citr != (*itr1)->GetGameObject()->GetComponentVec()->end();
//						citr++)
//					{
//						(*citr)->OnCollisionExit((*itr3));
//					}
//				}
//			}
//		}
//	}
//
//
//	for (int i = 0; i < 4; i++)
//	{
//		delete rctDivision[i];
//	}
//	delete[] rctDivision;
//}
//
//void CollisionManager::Update()
//{
//	RECT rect;
//	//GetClientRect(WindowFrame::GetInstance()->GetHWND(), &rect);
//	//rect.left += Camera::GetInstance()->GetPos().x;
//	//rect.right += Camera::GetInstance()->GetPos().x;
//	//rect.top += Camera::GetInstance()->GetPos().y;
//	//rect.bottom += Camera::GetInstance()->GetPos().y;
//
//	rect.left = -5000;
//	rect.right = 5000;
//	rect.top = 5000;
//	rect.bottom = -5000;
//
//	CollisionUpdate(m_objVec, rect, 10);
//}
//
//void CollisionManager::Clear()
//{
//	m_objVec->clear();
//}
//
//int CollisionManager::Count()
//{
//	return m_objVec->size();
//}
