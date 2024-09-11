#include "pch.h"
#include "GameObject.h"
#include "DebugWindow.h"
GameObject::GameObject() {
    m_vecComponent = new vector<Component*>();
    m_children = new vector<GameObject*>();
}

GameObject::~GameObject() {
}

const D3DXVECTOR3& GameObject::Position() {
    return m_position;
}

void GameObject::SetPosition(D3DXVECTOR3 v)
{
    D3DXVECTOR3 d = {v.x - m_position.x,  v.y - m_position.y,  v.z - m_position.z };
	m_position.x = v.x;
	m_position.y = v.y;
    m_position.z = v.z;
    if (m_box != nullptr)
        m_box->GetBody()->SetTransform({ m_position.x, m_position.y }, m_box->GetBody()->GetAngle());
    
	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
		(*itr)->AddPosition(d);
}

void GameObject::AddPosition(D3DXVECTOR3 v)
{
	m_position.x += v.x;
	m_position.y += v.y;
    m_position.z += v.z;
    if (m_box != nullptr)
        m_box->GetBody()->SetTransform({ m_position.x, m_position.y }, m_box->GetBody()->GetAngle());

	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
		(*itr)->AddPosition(v);
}

D3DXVECTOR2 GameObject::Size2D() 
{
    D3DXVECTOR2 size2d = { m_size.x, m_size.y };
	return size2d;
}

D3DXVECTOR3& GameObject::Size3D()
{
    return m_size;
}

void GameObject::SetDestroy(bool destroy) {
    m_isDestroy = destroy;
}

bool GameObject::GetDestroy() {
    return m_isDestroy;
}

void GameObject::SetTag(string tag) {
    m_tag = tag;
}

string GameObject::GetTag() {
    return m_tag;
}

void GameObject::SetActive(bool isActive) {
	m_setActive = isActive;
}

bool GameObject::GetActive() {
	return m_setActive;
}

Component* GameObject::AddComponent(Component* component) {
    if (component) {
        component->InitGameObj(this);
        m_vecComponent->push_back(component);
        component->Initialize();
        if (ObjectManager::GetInstance()->FindObject(this)) {
            component->Start();
        }
        return component;
    }
    return nullptr;
}

void GameObject::DeleteComponent(Component* component) {
    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++) {
        if ((*itr) == component) {
            (*itr)->Release();
            delete (*itr);
            (*itr) = nullptr;
            m_vecComponent->erase(itr);
            return;
        }
    }
}

vector<Component*>* GameObject::GetComponentVec() {
    return m_vecComponent;
}

void GameObject::InitializeSet() {
    m_isDestroy = false;
    m_box = GetComponent<BoxCollider>();
	ObjectManager::GetInstance()->AddObject(this);
}

void GameObject::Release() {

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
        if ((*itr) != nullptr)
        {
            (*itr)->Release();
            delete(*itr);
            (*itr) = nullptr;
        }
	}
	delete m_vecComponent;
    m_vecComponent = nullptr;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
    {
        (*itr)->SetParent(nullptr);
        (*itr)->SetDestroy(true);
    }
    delete m_children;
    m_children = nullptr;

    if (m_parent != nullptr)
        m_parent->DeleteChild(this);
}

void GameObject::Start() {
    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
        (*itr)->Start();
}

void GameObject::Update() {
    if (!m_setActive)
        return;
    if (m_isDestroy)
        return;
    RECT rect = {
        m_position.x + Camera::GetInstance()->GetPos().x,
        m_position.y + Camera::GetInstance()->GetPos().y,
        m_position.x + Camera::GetInstance()->GetPos().x + m_size.x,
		m_position.y + Camera::GetInstance()->GetPos().y + m_size.y };
	POINT point = { 
        Mouse::GetInstance()->GetDXPos().x + Camera::GetInstance()->GetPos().x, 
        Mouse::GetInstance()->GetDXPos().y + Camera::GetInstance()->GetPos().y };

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		(*itr)->Update();
		if (PtInRect(&rect, point))
		{
			(*itr)->OnMouseHover();
		}
	}
}

void GameObject::SetParent(GameObject* obj)
{
    m_parent = obj;
    if (obj == nullptr)
        return;
    obj->m_children->push_back(this);
}

void GameObject::AddChild(GameObject* obj)
{
    m_children->push_back(obj);
    obj->m_parent = this;
}

void GameObject::DeleteChild(GameObject* obj)
{
    if (m_children == nullptr)
        return;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
    {
        if ((*itr) == obj)
        {
            m_children->erase(itr);
            return;
        }
    }
}

const float& GameObject::GetAngleZ()
{
    return m_angleZ;
}

const float& GameObject::GetAngleX()
{
    return m_angleX;
}

const float& GameObject::GetAngleY()
{
    return m_angleY;
}

void GameObject::SetAngleZ(float v)
{
    m_angleZ = v;
    if (m_box != nullptr)
        m_box->GetBody()->SetTransform(m_box->GetBody()->GetPosition(), v);

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleZ(v);
}

void GameObject::SetAngleX(float v)
{
    m_angleX = v;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleX(v);
}

void GameObject::SetAngleY(float v)
{
    m_angleY = v;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleY(v);
}

void GameObject::OnCollisionEnter(Collider* col)
{
    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnCollisionEnter(col);
    }
}

void GameObject::OnCollisionStay(Collider* col)
{
	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		(*itr)->OnCollision(col);
	}
}

void GameObject::OnCollisionExit(Collider* col)
{
	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		if (*itr)
			(*itr)->OnCollisionExit(col);
	}
}

void GameObject::OnLBtnDown()
{
    if (!m_setActive || !m_vecComponent)
        return;

    RECT rect = {
         m_position.x + Camera::GetInstance()->GetPos().x,
         m_position.y + Camera::GetInstance()->GetPos().y,
         m_position.x + Camera::GetInstance()->GetPos().x + m_size.x,
         m_position.y + Camera::GetInstance()->GetPos().y + m_size.y };
    POINT point = { 
        Mouse::GetInstance()->GetDXPos().x + Camera::GetInstance()->GetPos().x,
        Mouse::GetInstance()->GetDXPos().y + Camera::GetInstance()->GetPos().y};

    if (PtInRect(&rect, point))
    {
        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
            (*itr)->OnLBtnDown();
    }
}

void GameObject::OnLBtnUp()
{
    if (!m_setActive || !m_vecComponent)
        return;

    RECT rect = {
        m_position.x + Camera::GetInstance()->GetPos().x,
        m_position.y + Camera::GetInstance()->GetPos().y,
        m_position.x + Camera::GetInstance()->GetPos().x + m_size.x,
        m_position.y + Camera::GetInstance()->GetPos().y + m_size.y };
    POINT point = { 
        Mouse::GetInstance()->GetDXPos().x + Camera::GetInstance()->GetPos().x,
        Mouse::GetInstance()->GetDXPos().y + Camera::GetInstance()->GetPos().y };

	if (PtInRect(&rect, point))
	{
		for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
		    	(*itr)->OnLBtnUp();
	}
}

void GameObject::OnRBtnDown()
{
    if (!m_setActive || !m_vecComponent)
        return;

    RECT rect = {
        m_position.x + Camera::GetInstance()->GetPos().x,
        m_position.y + Camera::GetInstance()->GetPos().y,
        m_position.x + Camera::GetInstance()->GetPos().x + m_size.x,
        m_position.y + Camera::GetInstance()->GetPos().y + m_size.y };
    POINT point = { 
        Mouse::GetInstance()->GetDXPos().x + Camera::GetInstance()->GetPos().x,
        Mouse::GetInstance()->GetDXPos().y + Camera::GetInstance()->GetPos().y};

    if (PtInRect(&rect, point))
    {
        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
            (*itr)->OnRBtnDown();
    }
}

void GameObject::OnRBtnUp()
{
    if (!m_setActive || !m_vecComponent)
        return;

    RECT rect = {
         m_position.x + Camera::GetInstance()->GetPos().x,
         m_position.y + Camera::GetInstance()->GetPos().y,
         m_position.x + Camera::GetInstance()->GetPos().x + m_size.x,
         m_position.y + Camera::GetInstance()->GetPos().y + m_size.y };
    POINT point = { 
        Mouse::GetInstance()->GetDXPos().x + Camera::GetInstance()->GetPos().x,
        Mouse::GetInstance()->GetDXPos().y + Camera::GetInstance()->GetPos().y};

    if (PtInRect(&rect, point))
    {
        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
            (*itr)->OnRBtnUp();
    }
}
