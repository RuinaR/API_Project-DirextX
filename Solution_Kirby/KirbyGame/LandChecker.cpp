//#include "pch.h"
//#include "LandChecker.h"
//
//void LandChecker::Initialize()
//{
//	m_box = m_gameObj->GetComponent<BoxCollider>();
//	if (m_box == nullptr)
//	{
//		m_box = new BoxCollider(b2BodyType::b2_kinematicBody);
//		m_box->SetTrigger(true);
//		m_gameObj->AddComponent(m_box);
//	}
//	m_gameObj->SetTag("Checker");
//}
//
//void LandChecker::Release()
//{
//}
//
//void LandChecker::Start()
//{
//}
//
//void LandChecker::Update()
//{
//}
//
//void LandChecker::CollisionEnter(Collider* other)
//{
//	//if (other->GetGameObject()->GetTag() == TAG_LAND)
//	//{
//	//	m_isOnLand = true;
//	//}
//}
//
//void LandChecker::CollisionExit(Collider* other)
//{
//	/*for (set<Collider*>::iterator itr = m_box->SetCol()->begin(); itr != m_box->SetCol()->end(); itr++)
//	{
//		if ((*itr)->GetGameObject()->GetTag() == TAG_LAND)
//		{
//			m_isOnLand = true;
//			return;
//		}
//	}
//	m_isOnLand = false;*/
//}
//
//void LandChecker::Collision(Collider* other)
//{
//
//}
//
//BoxCollider* LandChecker::GetCollider()
//{
//	return m_box;
//}
//
//bool LandChecker::IsOnLand()
//{
//	return m_isOnLand;
//}
