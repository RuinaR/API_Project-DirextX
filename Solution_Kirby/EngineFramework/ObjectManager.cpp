#include "pch.h"
#include "GameObject.h"
#include "Component.h"
#include "DebugWindow.h"
ObjectManager* ObjectManager::m_Pthis = nullptr;

bool ObjectManager::IsInObjectList(GameObject* obj)
{
	if (obj == nullptr || m_objList == nullptr)
		return false;

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

bool ObjectManager::IsPendingAdd(GameObject* obj)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

bool ObjectManager::IsPendingRemove(GameObject* obj)
{
	for (vector<GameObject*>::iterator itr = m_pendingRemoveObjects.begin(); itr != m_pendingRemoveObjects.end(); itr++)
	{
		if ((*itr) == obj)
			return true;
	}
	return false;
}

void ObjectManager::QueueDestroyObject(GameObject* obj)
{
	if (obj == nullptr || IsPendingRemove(obj))
		return;

	obj->SetDestroy(true);
	m_pendingRemoveObjects.push_back(obj);
}

void ObjectManager::ReleaseAndDeleteObject(GameObject* obj)
{
	if (obj == nullptr)
		return;

	if (m_selected == obj)
		m_selected = nullptr;

	obj->Release();
	delete obj;
}

void ObjectManager::ProcessChildNode(GameObject* obj, int depth)
{
	if (obj == nullptr || obj->GetDestroy())
		return;

	char str[64];
	string d = "  ";
	for (int i = 0; i < depth; i++)
	{
		d.append(" ");
	}
	d.append("L");
	for (auto node = obj->GetChild()->begin(); node != obj->GetChild()->end(); node++)
	{
		if ((*node) == nullptr || (*node)->GetDestroy())
			continue;

		sprintf_s(str, 64, "%s %s", d.c_str(), (*node)->GetTag().c_str());
		ProcessChildNode(*node, depth + 1);
		if (ImGui::Selectable(str, m_selected == (*node)))
		{
			m_selected = *node;
		}
	}
}

void ObjectManager::ImguiUpdate()
{
	char str[64];

	ImGui::Begin("Hierarchy");
	int i = 0;
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;

		i++;
		if ((*itr)->GetParent() != nullptr)
			continue;
		sprintf_s(str, 64, "-%d. %s", i, (*itr)->GetTag().c_str());
		if (ImGui::Selectable(str, m_selected == (*itr)))
		{
			m_selected = (*itr);
		}
		ProcessChildNode((*itr), 1);
	}
	ImGui::End();

	if (m_selected != nullptr && !m_selected->GetDestroy())
	{
		ImGui::Begin("Inspector");

		sprintf_s(str, 64, "X:%.2f, Y:%.2f, Z:%.2f", m_selected->Position().x, m_selected->Position().y, m_selected->Position().z);
		ImGui::Text(str);
		for (vector<Component*>::iterator itr = m_selected->GetComponentVec()->begin(); itr != m_selected->GetComponentVec()->end(); itr++)
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
		for (vector<GameObject*>::iterator itr = m_Pthis->m_pendingAddObjects.begin(); itr != m_Pthis->m_pendingAddObjects.end(); itr++)
		{
			m_Pthis->ReleaseAndDeleteObject(*itr);
		}
		m_Pthis->m_pendingAddObjects.clear();
		m_Pthis->m_pendingRemoveObjects.clear();

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
	if (obj == nullptr || IsInObjectList(obj) || IsPendingAdd(obj) || IsPendingRemove(obj))
		return;

	m_pendingAddObjects.push_back(obj);
}

bool ObjectManager::UnregisterObject(GameObject* obj)
{
	if (obj == nullptr)
		return false;

	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) == obj)
		{
			m_pendingAddObjects.erase(itr);
			return true;
		}
	}

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
	AddObject(obj);
}

bool ObjectManager::DestroyObject(GameObject* obj)
{
	if (obj == nullptr || IsPendingRemove(obj))
		return false;

	if (!IsInObjectList(obj) && !IsPendingAdd(obj))
		return false;

	QueueDestroyObject(obj);
	return true;
}

bool ObjectManager::DestroyObject(string tag)
{
	for (vector<GameObject*>::iterator itr = m_pendingAddObjects.begin(); itr != m_pendingAddObjects.end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			QueueDestroyObject(*itr);
			return true;
		}
	}

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && (*itr)->GetTag() == tag)
		{
			QueueDestroyObject(*itr);
			return true;
		}
	}
	return false;
}

GameObject* ObjectManager::FindObject(string tag)
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr && !(*itr)->GetDestroy() && (*itr)->GetTag() == tag)
		{
			return (*itr);
		}
	}
	return nullptr;
}

bool ObjectManager::FindObject(GameObject* obj)
{
	return IsInObjectList(obj) && !IsPendingRemove(obj) && obj != nullptr && !obj->GetDestroy();
}

void ObjectManager::Initialize()
{
	m_objList = new list<GameObject*>();
	m_selected = nullptr;
}

void ObjectManager::Release()
{
	FlushPendingObjects();

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) != nullptr)
			(*itr)->Release();
	}
}

void ObjectManager::Update()
{
	FlushPendingObjects();

	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr)
			continue;

		if ((*itr)->GetDestroy())
		{
			QueueDestroyObject(*itr);
			continue;
		}

		(*itr)->Update();

		if (((*itr)->GetDestroy()))
		{
			QueueDestroyObject(*itr);
		}
	}

	FlushPendingObjects();
}

void ObjectManager::FlushPendingObjects()
{
	if (m_isFlushing)
		return;

	m_isFlushing = true;

	vector<GameObject*> pendingAddObjects;
	pendingAddObjects.swap(m_pendingAddObjects);

	for (vector<GameObject*>::iterator itr = pendingAddObjects.begin(); itr != pendingAddObjects.end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr)
			continue;

		if (!IsInObjectList(obj))
			m_objList->push_back(obj);

		if (!obj->GetDestroy())
			obj->Start();
	}

	vector<GameObject*> pendingRemoveObjects;
	pendingRemoveObjects.swap(m_pendingRemoveObjects);

	for (vector<GameObject*>::iterator itr = pendingRemoveObjects.begin(); itr != pendingRemoveObjects.end(); itr++)
	{
		GameObject* obj = *itr;
		if (obj == nullptr)
			continue;

		for (list<GameObject*>::iterator listItr = m_objList->begin(); listItr != m_objList->end();)
		{
			if ((*listItr) == obj)
			{
				listItr = m_objList->erase(listItr);
				break;
			}
			else
			{
				listItr++;
			}
		}

		ReleaseAndDeleteObject(obj);
	}

	m_isFlushing = false;
}

void ObjectManager::Clear()
{
	if (m_Pthis)
	{
		for (list<GameObject*>::iterator itr = m_Pthis->m_objList->begin(); itr != m_Pthis->m_objList->end(); itr++)
		{
			m_Pthis->QueueDestroyObject(*itr);
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
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnLBtnDown();
	}
}

void ObjectManager::OnLBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnLBtnUp();
	}
}

void ObjectManager::OnRBtnDown()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnRBtnDown();
	}
}

void ObjectManager::OnRBtnUp()
{
	for (list<GameObject*>::iterator itr = m_objList->begin(); itr != m_objList->end(); itr++)
	{
		if ((*itr) == nullptr || (*itr)->GetDestroy())
			continue;
		(*itr)->OnRBtnUp();
	}
}
