#include "pch.h"
#include "GameObject.h"
#include "DebugWindow.h"
ObjectManager* ObjectManager::m_Pthis = nullptr;

void ObjectManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new ObjectManager();
		m_Pthis->m_objList = new list<GameObject*>();

	}
}

ObjectManager* ObjectManager::GetInstance()
{
	return m_Pthis;
}

void ObjectManager::Destroy()
{
	if (m_Pthis)
	{
		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			delete(*itr);
			(*itr) = nullptr;
		}
		delete m_Pthis->m_objList;
		m_Pthis->m_objList = nullptr;

		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void ObjectManager::AddObject(GameObject* obj)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr)->GetOrderInLayer() > obj->GetOrderInLayer())
		{
			m_objList->insert(itr, obj);
			obj->Start();
			return;
		}
	}
	m_objList->push_back(obj);
	obj->Start();
}

bool ObjectManager::UnregisterObject(GameObject* obj)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
		{
			m_objList->erase(itr);
			return true;
		}
	}
	return false;
}

void ObjectManager::RegisterObject(GameObject* obj)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr)->GetOrderInLayer() > obj->GetOrderInLayer())
		{
			m_objList->insert(itr, obj);
			return;
		}
	}
	m_objList->push_back(obj);
}

bool ObjectManager::DestroyObject(GameObject* obj)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
		{
			(*itr)->Release();
			delete(*itr);
			(*itr) = nullptr;
			m_objList->erase(itr);
			return true;
		}
	}
	return false;
}

bool ObjectManager::DestroyObject(string tag)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr)->GetTag() == tag)
		{
			(*itr)->Release();
			delete(*itr);
			(*itr) = nullptr;
			m_objList->erase(itr);
			return true;
		}
	}
	return false;
}

GameObject* ObjectManager::FindObject(string tag)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr)->GetTag() == tag)
		{
			return (*itr);
		}
	}
	return nullptr;
}

bool ObjectManager::FindObject(GameObject* obj)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
		{
			return true;
		}
	}
	return false;
}

void ObjectManager::Release()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		(*itr)->Release();
	}
}

void ObjectManager::Update()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end();)
	{
		(*itr)->Update();
		if (((*itr)->GetDestroy()))
		{
			(*itr)->Release();
			delete(*itr);
			(*itr) = nullptr;
			list<GameObject*>::iterator itrTmp = itr;
			itrTmp++;
			m_objList->erase(itr);
			itr = itrTmp;
		}
		else
			itr++;
	}
}

void ObjectManager::Clear()
{
	if (m_Pthis)
	{
		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			(*itr)->SetDestroy(true);
		}
	}
}

int ObjectManager::Count()
{
	return m_objList->size();
}

list<GameObject*>* ObjectManager::GetObjList()
{
	return m_objList;
}

void ObjectManager::OnLBtnDown()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		(*itr)->OnLBtnDown();
	}
}

void ObjectManager::OnLBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		(*itr)->OnLBtnUp();
	}
}

void ObjectManager::OnRBtnDown()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		(*itr)->OnRBtnDown();
	}
}

void ObjectManager::OnRBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		(*itr)->OnRBtnUp();
	}
}
