#include "pch.h"
#include "GameObject.h"
#include "DebugWindow.h"
#include "Rigidbody2D.h"
#include "SceneJsonUtility.h"

namespace
{
	b2Body* GetPrimaryPhysicsBody(GameObject* gameObject, BoxCollider* boxCollider)
	{
		if (gameObject == nullptr)
		{
			return nullptr;
		}

		Rigidbody2D* rigidbody2D = gameObject->GetComponent<Rigidbody2D>();
		if (rigidbody2D != nullptr && rigidbody2D->GetBody() != nullptr)
		{
			return rigidbody2D->GetBody();
		}

		if (boxCollider != nullptr)
		{
			return boxCollider->GetBody();
		}

		return nullptr;
	}

	void SyncBodyTransform(b2Body* body, const D3DXVECTOR3& position, float angle)
	{
		if (body == nullptr)
		{
			return;
		}

		body->SetTransform({ position.x, position.y }, angle);
		if (body->GetType() != b2_staticBody)
		{
			body->SetAwake(true);
		}
	}

	bool CanDispatchMouseEvent(GameObject* gameObject)
	{
		// 마우스 Down/Up/Hover Stay는 delete-pending 오브젝트에 전달하지 않는다.
		return gameObject != nullptr &&
			gameObject->GetComponentVec() != nullptr &&
			gameObject->GetActive() &&
			!gameObject->GetDestroy();
	}
}

GameObject::GameObject() {
    m_vecComponent = new vector<Component*>();
    m_pendingDeleteComponents = new vector<Component*>();
    m_children = new vector<GameObject*>();
    m_angleX = 0.0f;
    m_angleY = 0.0f;
    m_angleZ = 0.0f;
    m_box = nullptr;
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
    b2Body* body = GetPrimaryPhysicsBody(this, m_box);
    SyncBodyTransform(body, m_position, body != nullptr ? body->GetAngle() : 0.0f);
    
	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
		(*itr)->AddPosition(d);
}

void GameObject::AddPosition(D3DXVECTOR3 v)
{
	m_position.x += v.x;
	m_position.y += v.y;
    m_position.z += v.z;
    b2Body* body = GetPrimaryPhysicsBody(this, m_box);
    SyncBodyTransform(body, m_position, body != nullptr ? body->GetAngle() : 0.0f);

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

int GameObject::GetId() const
{
    return m_objectId;
}

void GameObject::SetId(int objectId)
{
    m_objectId = objectId;
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

std::string GameObject::Serialize() const
{
    return Serialize(-1, -1);
}

std::string GameObject::Serialize(int objectId, int parentId) const
{
    return Serialize(objectId, parentId, 3);
}

std::string GameObject::Serialize(int objectId, int parentId, int sceneVersion) const
{
    std::ostringstream oss;
    oss << "{\n";
    if (sceneVersion >= 4)
    {
        if (objectId >= 0)
        {
            oss << "      \"id\": " << objectId << ",\n";
            oss << "      \"parentId\": " << parentId << ",\n";
        }
        oss << "      \"name\": \"" << SceneJson::EscapeString(m_tag) << "\",\n";
        oss << "      \"tag\": \"" << SceneJson::EscapeString(m_tag) << "\",\n";
        oss << "      \"active\": " << (m_setActive ? "true" : "false") << ",\n";
        oss << "      \"transform\": {\n";
        oss << "        \"position\": { \"x\": " << m_position.x << ", \"y\": " << m_position.y << ", \"z\": " << m_position.z << " },\n";
        oss << "        \"rotation\": { \"x\": " << m_angleX << ", \"y\": " << m_angleY << ", \"z\": " << m_angleZ << " },\n";
        oss << "        \"size\": { \"x\": " << m_size.x << ", \"y\": " << m_size.y << ", \"z\": " << m_size.z << " }\n";
        oss << "      },\n";
    }
    else
    {
        if (objectId >= 0)
        {
            oss << "      \"objectId\": " << objectId << ",\n";
            oss << "      \"parentId\": " << parentId << ",\n";
        }
        oss << "      \"tag\": \"" << SceneJson::EscapeString(m_tag) << "\",\n";
        oss << "      \"active\": " << (m_setActive ? "true" : "false") << ",\n";
        oss << "      \"position\": { \"x\": " << m_position.x << ", \"y\": " << m_position.y << ", \"z\": " << m_position.z << " },\n";
        oss << "      \"size\": { \"x\": " << m_size.x << ", \"y\": " << m_size.y << ", \"z\": " << m_size.z << " },\n";
        oss << "      \"angle\": { \"x\": " << m_angleX << ", \"y\": " << m_angleY << ", \"z\": " << m_angleZ << " },\n";
    }
    oss << "      \"components\": [";

    bool isFirstComponent = true;
    if (m_vecComponent != nullptr)
    {
        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
        {
            Component* component = *itr;
            if (component == nullptr || strlen(component->GetSerializableType()) == 0)
                continue;

            if (!isFirstComponent)
            {
                oss << ",";
            }
            oss << "\n";
            oss << "        { \"type\": \"" << component->GetSerializableType() << "\", \"data\": " << component->Serialize() << " }";
            isFirstComponent = false;
        }
    }

    if (!isFirstComponent)
    {
        oss << "\n      ";
    }
    oss << "]\n";
    oss << "    }";
    return oss.str();
}

bool GameObject::Deserialize(const std::string& objectJson, GameObjectSerializedData& outData)
{
    return Deserialize(objectJson, outData, 3);
}

bool GameObject::Deserialize(const std::string& objectJson, GameObjectSerializedData& outData, int sceneVersion)
{
    GameObjectSerializedData data;
    if (sceneVersion >= 4)
    {
        std::string transformJson;
        const bool hasName = SceneJson::ReadString(objectJson, "name", data.name);
        const bool hasTag = SceneJson::ReadString(objectJson, "tag", data.tag);
        SceneJson::ReadInt(objectJson, "id", data.objectId);
        SceneJson::ReadInt(objectJson, "parentId", data.parentId);
        if (!hasTag)
        {
            data.tag = data.name;
        }
        if (!hasName && !hasTag)
        {
            return false;
        }
        if (data.name.empty())
        {
            data.name = data.tag;
        }
        if (!SceneJson::ReadBool(objectJson, "active", data.active))
            return false;
        if (!SceneJson::ExtractObject(objectJson, "transform", transformJson))
            return false;
        if (!SceneJson::ReadVector3(transformJson, "position", &data.position))
            return false;
        if (!SceneJson::ReadVector3(transformJson, "size", &data.size))
            return false;
        if (!SceneJson::ReadVector3(transformJson, "rotation", &data.angle))
            return false;
    }
    else
    {
        SceneJson::ReadInt(objectJson, "objectId", data.objectId);
        SceneJson::ReadInt(objectJson, "parentId", data.parentId);
        if (!SceneJson::ReadString(objectJson, "tag", data.tag))
            return false;
        data.name = data.tag;
        if (!SceneJson::ReadBool(objectJson, "active", data.active))
            return false;
        if (!SceneJson::ReadVector3(objectJson, "position", &data.position))
		    return false;
        if (!SceneJson::ReadVector3(objectJson, "size", &data.size))
		    return false;
        if (!SceneJson::ReadVector3(objectJson, "angle", &data.angle))
		    return false;
    }

    outData = data;
    return true;
}

GameObject* GameObject::CreateFromSerializedData(const GameObjectSerializedData& data)
{
    GameObject* obj = new GameObject();
    obj->SetId(data.objectId);
    obj->SetTag(data.tag);
    obj->SetActive(data.active);
    obj->SetPosition(data.position);
    obj->Size3D() = data.size;
    obj->SetAngleX(data.angle.x);
    obj->SetAngleY(data.angle.y);
    obj->SetAngleZ(data.angle.z);
    return obj;
}

Component* GameObject::AddComponent(Component* component) {
    return AddComponent(component, true, true);
}

Component* GameObject::AddComponent(Component* component, bool initializeComponent, bool startComponent) {
    if (component) {
        component->InitGameObj(this);
        m_vecComponent->push_back(component);
        if (BoxCollider* boxCollider = dynamic_cast<BoxCollider*>(component))
        {
            m_box = boxCollider;
        }
        if (initializeComponent)
            component->Initialize();
        if (startComponent && ObjectManager::GetInstance()->FindObject(this)) {
            component->Start();
        }
        return component;
    }
    return nullptr;
}

void GameObject::DeleteComponent(Component* component) {
    if (component == nullptr || m_vecComponent == nullptr)
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++) {
        if ((*itr) == component) {
            Component* target = *itr;
            if (target == m_box)
            {
                m_box = nullptr;
            }
            m_vecComponent->erase(itr);
            if (m_pendingDeleteComponents != nullptr)
            {
                m_pendingDeleteComponents->push_back(target);
            }
            else
            {
                target->Release();
                delete target;
            }
            return;
        }
    }
}

vector<Component*>* GameObject::GetComponentVec() {
    return m_vecComponent;
}

void GameObject::FlushPendingComponents()
{
    if (m_pendingDeleteComponents == nullptr || m_pendingDeleteComponents->empty())
        return;

    vector<Component*> pendingDeleteComponents;
    pendingDeleteComponents.swap(*m_pendingDeleteComponents);

    for (vector<Component*>::iterator itr = pendingDeleteComponents.begin(); itr != pendingDeleteComponents.end(); itr++)
    {
        Component* component = *itr;
        if (component == nullptr)
            continue;

        component->Release();
        delete component;
    }
}

void GameObject::InitializeSet() {
    m_isDestroy = false;
    m_box = GetComponent<BoxCollider>();
	ObjectManager::GetInstance()->AddObject(this);
}

void GameObject::Release() {

    FlushPendingComponents();

	if (m_vecComponent != nullptr)
	{
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
	}

    if (m_pendingDeleteComponents != nullptr)
    {
        delete m_pendingDeleteComponents;
        m_pendingDeleteComponents = nullptr;
    }

	if (m_children != nullptr)
	{
        while (!m_children->empty())
        {
            GameObject* child = m_children->back();
            m_children->pop_back();
            if (child != nullptr)
            {
                if (child->m_parent == this)
                    child->m_parent = nullptr;
                child->SetDestroy(true);
            }
        }
		delete m_children;
		m_children = nullptr;
	}

    if (m_parent != nullptr)
    {
        m_parent->DeleteChild(this);
        m_parent = nullptr;
    }
}

void GameObject::Start() {
    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
        (*itr)->Start();
}

void GameObject::Update() {
    FlushPendingComponents();

    if (!m_setActive)
        return;
    if (m_isDestroy)
        return;

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		(*itr)->Update();
	}
}

void GameObject::SetParent(GameObject* obj)
{
    if (obj == this)
        return;

    for (GameObject* parent = obj; parent != nullptr; parent = parent->GetParent())
    {
        if (parent == this)
            return;
    }

    if (m_parent == obj)
        return;

    if (m_parent != nullptr)
    {
        m_parent->DeleteChild(this);
    }

    m_parent = obj;
    if (obj == nullptr)
        return;
    obj->m_children->push_back(this);
}

void GameObject::AddChild(GameObject* obj)
{
    if (obj == nullptr || obj == this)
        return;

    for (GameObject* parent = this; parent != nullptr; parent = parent->GetParent())
    {
        if (parent == obj)
            return;
    }

    if (obj->m_parent == this)
        return;

    if (obj->m_parent != nullptr)
    {
        obj->m_parent->DeleteChild(obj);
    }

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
    const float delta = v - m_angleZ;
    m_angleZ = v;
    b2Body* body = GetPrimaryPhysicsBody(this, m_box);
    if (body != nullptr)
    {
        body->SetTransform(body->GetPosition(), v);
        if (body->GetType() != b2_staticBody)
        {
            body->SetAwake(true);
        }
    }

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleZ((*itr)->GetAngleZ() + delta);
}

void GameObject::SetAngleX(float v)
{
    const float delta = v - m_angleX;
    m_angleX = v;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleX((*itr)->GetAngleX() + delta);
}

void GameObject::SetAngleY(float v)
{
    const float delta = v - m_angleY;
    m_angleY = v;

    for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
        (*itr)->SetAngleY((*itr)->GetAngleY() + delta);
}

void GameObject::OnCollisionEnter(Collider* col)
{
    if (m_vecComponent == nullptr || m_isDestroy)
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnCollisionEnter(col);
    }
}

void GameObject::OnCollisionStay(Collider* col)
{
    if (m_vecComponent == nullptr || m_isDestroy)
        return;

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		(*itr)->OnCollisionStay(col);
	}
}

void GameObject::OnCollisionExit(Collider* col)
{
    if (m_vecComponent == nullptr)
        return;

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		if (*itr)
			(*itr)->OnCollisionExit(col);
	}
}

void GameObject::OnLBtnDown()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnLBtnDown();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}

void GameObject::OnLBtnUp()
{
    if (!CanDispatchMouseEvent(this))
        return;

	for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
	{
		(*itr)->OnLBtnUp();
		// 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
		if (m_isDestroy)
			break;
	}
}

void GameObject::OnRBtnDown()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnRBtnDown();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}

void GameObject::OnRBtnUp()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnRBtnUp();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}

void GameObject::OnMouseHoverEnter()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnMouseHoverEnter();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}

void GameObject::OnMouseHoverStay()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnMouseHoverStay();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}

void GameObject::OnMouseHoverExit()
{
    if (!CanDispatchMouseEvent(this))
        return;

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        (*itr)->OnMouseHoverExit();
        // 이벤트 처리 중 자기 자신이 삭제 예약되면 남은 Component 전파를 중단한다.
        if (m_isDestroy)
            break;
    }
}
