#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "DebugWindow.h"
ObjectManager* ObjectManager::m_Pthis = nullptr;

void ObjectManager::ImguiUpdate()
{
	char str[64];

	ImGui::Begin("Hierarchy");
	int i = 0;
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		i++;
		sprintf_s(str, 64, "-%d. %s", i, (*itr)->GetTag().c_str());
		if (ImGui::Selectable(str, m_selected == itr))
		{
			m_selected = itr;
		}
	}
	ImGui::End();

	if (m_selected != m_objList->end() && !(*m_selected)->GetDestroy())
	{
		ImGui::Begin("Inspector");

		sprintf_s(str, 64, "X:%.2f, Y:%.2f, Z:%.2f", (*m_selected)->Position().x, (*m_selected)->Position().y, (*m_selected)->Position().z);
		ImGui::Text(str);
		for (vector<Component*>::iterator itr = (*m_selected)->GetComponentVec()->begin(); itr != (*m_selected)->GetComponentVec()->end(); itr++)
		{
			const type_info& typeInfo = typeid(**itr);
			ImGui::Text(typeInfo.name());
		}

		ImGui::End();
	}
}

void ObjectManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new ObjectManager();
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
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void ObjectManager::AddObject(GameObject* obj)
{
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

void ObjectManager::Initialize()
{
	m_objList = new list<GameObject*>();
	m_selected = m_objList->end();
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
			if (m_selected == itr)
				m_selected = m_objList->end();
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
	//ImguiUpdate();
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
