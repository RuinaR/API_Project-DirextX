#include "pch.h"
#include "GameObject.h"
#include "DebugWindow.h"
#include "Rigidbody2D.h"
#include "Collider2D.h"
#include "SceneJsonUtility.h"

namespace
{
	bool IsComponentStillAttached(GameObject* gameObject, Component* component)
	{
		if (gameObject == nullptr || component == nullptr)
		{
			return false;
		}

		vector<Component*>* components = gameObject->GetComponentVec();
		if (components == nullptr)
		{
			return false;
		}

		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			if (*itr == component)
			{
				return true;
			}
		}

		return false;
	}

	template <typename Dispatcher>
	void DispatchComponentEventSnapshot(GameObject* gameObject, Dispatcher&& dispatcher, bool stopOnDestroy)
	{
		if (gameObject == nullptr || gameObject->GetComponentVec() == nullptr)
		{
			return;
		}

		vector<Component*> snapshot = *gameObject->GetComponentVec();
		for (vector<Component*>::iterator itr = snapshot.begin(); itr != snapshot.end(); itr++)
		{
			Component* component = *itr;
			if (component == nullptr || !IsComponentStillAttached(gameObject, component))
			{
				continue;
			}

			dispatcher(component);
			if (stopOnDestroy && gameObject->GetDestroy())
			{
				break;
			}
		}
	}

	b2Body* GetPrimaryPhysicsBody(GameObject* gameObject)
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

		vector<Component*>* components = gameObject->GetComponentVec();
		if (components != nullptr)
		{
			for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
			{
				Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
				if (collider != nullptr && collider->GetBody() != nullptr)
				{
					return collider->GetBody();
				}
			}
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

	bool IsPhysicsWorldLocked()
	{
		MainFrame* mainFrame = MainFrame::GetInstance();
		if (mainFrame == nullptr || mainFrame->GetBox2dWorld() == nullptr)
		{
			return false;
		}

		return mainFrame->GetBox2dWorld()->IsLocked();
	}

	D3DXMATRIX BuildEulerRotationMatrix(const D3DXVECTOR3& angle)
	{
		D3DXMATRIX rotationX;
		D3DXMATRIX rotationY;
		D3DXMATRIX rotationZ;
		D3DXMATRIX rotation;
		D3DXMatrixRotationX(&rotationX, angle.x);
		D3DXMatrixRotationY(&rotationY, angle.y);
		D3DXMatrixRotationZ(&rotationZ, angle.z);
		rotation = rotationZ * rotationX * rotationY;
		return rotation;
	}

	float SafeDivideScale(float value, float parentValue)
	{
		if (fabsf(parentValue) <= 0.0001f)
		{
			return value;
		}

		return value / parentValue;
	}
}

GameObject::GameObject() {
    m_vecComponent = new vector<Component*>();
    m_pendingDeleteComponents = new vector<Component*>();
    m_children = new vector<GameObject*>();
	D3DXMatrixIdentity(&m_worldMatrix);
	m_sizeWriteProxy = m_localSize;
}

GameObject::~GameObject() {
}

const D3DXVECTOR3& GameObject::Position() {
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    return m_position;
}

void GameObject::SetPosition(D3DXVECTOR3 v)
{
    SyncExternalSizeWriteback();
    ApplyWorldPosition(v);
}

void GameObject::AddPosition(D3DXVECTOR3 v)
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
	ApplyWorldPosition(D3DXVECTOR3(
		m_position.x + v.x,
		m_position.y + v.y,
		m_position.z + v.z));
}

D3DXVECTOR2 GameObject::Size2D() 
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    D3DXVECTOR2 size2d = { m_size.x, m_size.y };
	return size2d;
}

D3DXVECTOR3& GameObject::Size3D()
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    m_sizeWriteProxy = m_size;
    m_hasExternalSizeWrite = true;
    return m_sizeWriteProxy;
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
    const bool wasActive = m_setActive;
	m_setActive = isActive;
    if (wasActive != m_setActive)
    {
        if (IsPhysicsWorldLocked())
        {
            m_hasPendingPhysicsActiveStateChange = true;
            m_pendingPhysicsActiveState = m_setActive;
        }
        else
        {
            ApplyPhysicsActiveState(m_setActive);
        }
    }

    if (!wasActive && m_setActive)
    {
        OnEnable();
    }
    else if (wasActive && !m_setActive)
    {
        OnDisable();
    }
}

bool GameObject::GetActive() {
	return m_setActive;
}

void GameObject::MarkTransformDirty()
{
	m_transformDirty = true;
}

void GameObject::MarkTransformDirtyRecursive()
{
	MarkTransformDirty();

	if (m_children == nullptr)
	{
		return;
	}

	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
	{
		if (*itr != nullptr)
		{
			(*itr)->MarkTransformDirtyRecursive();
		}
	}
}

void GameObject::EnsureTransformCurrent() const
{
	if (!m_transformDirty)
	{
		return;
	}

	if (m_parent != nullptr)
	{
		m_parent->SyncExternalSizeWriteback();
		m_parent->EnsureTransformCurrent();
	}

	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_localSize.x, m_localSize.y, m_localSize.z);
	D3DXMATRIX rotationMatrix = BuildEulerRotationMatrix(m_localAngle);
	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(&translationMatrix, m_localPosition.x, m_localPosition.y, m_localPosition.z);
	D3DXMATRIX localMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	if (m_parent != nullptr)
	{
		m_worldMatrix = localMatrix * m_parent->m_worldMatrix;
		m_position = D3DXVECTOR3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
		m_size = D3DXVECTOR3(
			m_localSize.x * m_parent->m_size.x,
			m_localSize.y * m_parent->m_size.y,
			m_localSize.z * m_parent->m_size.z);
		m_angleX = m_parent->m_angleX + m_localAngle.x;
		m_angleY = m_parent->m_angleY + m_localAngle.y;
		m_angleZ = m_parent->m_angleZ + m_localAngle.z;
	}
	else
	{
		m_worldMatrix = localMatrix;
		m_position = m_localPosition;
		m_size = m_localSize;
		m_angleX = m_localAngle.x;
		m_angleY = m_localAngle.y;
		m_angleZ = m_localAngle.z;
	}

	m_transformDirty = false;
}

void GameObject::SyncExternalSizeWriteback() const
{
	if (!m_hasExternalSizeWrite)
	{
		return;
	}

	EnsureTransformCurrent();
	if (m_sizeWriteProxy.x == m_size.x &&
		m_sizeWriteProxy.y == m_size.y &&
		m_sizeWriteProxy.z == m_size.z)
	{
		m_hasExternalSizeWrite = false;
		return;
	}

	const_cast<GameObject*>(this)->ApplyWorldSize(m_sizeWriteProxy);
	const_cast<GameObject*>(this)->m_hasExternalSizeWrite = false;
}

void GameObject::QueueHierarchyPhysicsTransformSync()
{
	m_hasPendingPhysicsTransformSync = true;

	if (m_children == nullptr)
	{
		return;
	}

	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
	{
		if (*itr != nullptr)
		{
			(*itr)->QueueHierarchyPhysicsTransformSync();
		}
	}
}

void GameObject::SyncHierarchyPhysicsTransform()
{
	SyncExternalSizeWriteback();
	EnsureTransformCurrent();

	Rigidbody2D* rigidbody2D = GetComponent<Rigidbody2D>();
	if (rigidbody2D != nullptr)
	{
		rigidbody2D->SyncBodyToGameObjectTransform();
	}

	if (m_vecComponent != nullptr)
	{
		for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
		{
			Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
			if (collider != nullptr)
			{
				collider->SyncBodyToGameObjectTransform();
			}
		}
	}

	m_hasPendingPhysicsTransformSync = false;

	if (m_children == nullptr)
	{
		return;
	}

	for (vector<GameObject*>::iterator itr = m_children->begin(); itr != m_children->end(); itr++)
	{
		if (*itr != nullptr)
		{
			(*itr)->SyncHierarchyPhysicsTransform();
		}
	}
}

void GameObject::FlushPendingTransformSync()
{
	if (!m_hasPendingPhysicsTransformSync)
	{
		return;
	}

	if (IsPhysicsWorldLocked())
	{
		return;
	}

	SyncHierarchyPhysicsTransform();
}

void GameObject::ApplyWorldPosition(const D3DXVECTOR3& worldPosition)
{
	if (m_parent != nullptr)
	{
		m_parent->SyncExternalSizeWriteback();
		m_parent->EnsureTransformCurrent();
		D3DXMATRIX inverseParentMatrix;
		if (D3DXMatrixInverse(&inverseParentMatrix, nullptr, &m_parent->m_worldMatrix) != nullptr)
		{
			D3DXVec3TransformCoord(&m_localPosition, &worldPosition, &inverseParentMatrix);
		}
		else
		{
			m_localPosition = worldPosition;
		}
	}
	else
	{
		m_localPosition = worldPosition;
	}

	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyWorldSize(const D3DXVECTOR3& worldSize)
{
	if (m_parent != nullptr)
	{
		m_parent->SyncExternalSizeWriteback();
		m_parent->EnsureTransformCurrent();
		m_localSize = D3DXVECTOR3(
			SafeDivideScale(worldSize.x, m_parent->m_size.x),
			SafeDivideScale(worldSize.y, m_parent->m_size.y),
			SafeDivideScale(worldSize.z, m_parent->m_size.z));
	}
	else
	{
		m_localSize = worldSize;
	}

	m_sizeWriteProxy = worldSize;
	m_hasExternalSizeWrite = false;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyWorldAngleX(float worldAngleX)
{
	m_localAngle.x = m_parent != nullptr ? worldAngleX - m_parent->GetAngleX() : worldAngleX;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyWorldAngleY(float worldAngleY)
{
	m_localAngle.y = m_parent != nullptr ? worldAngleY - m_parent->GetAngleY() : worldAngleY;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyWorldAngleZ(float worldAngleZ)
{
	m_localAngle.z = m_parent != nullptr ? worldAngleZ - m_parent->GetAngleZ() : worldAngleZ;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyLocalPosition(const D3DXVECTOR3& localPosition)
{
	m_localPosition = localPosition;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyLocalSize(const D3DXVECTOR3& localSize)
{
	m_localSize = localSize;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyLocalAngleX(float localAngleX)
{
	m_localAngle.x = localAngleX;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyLocalAngleY(float localAngleY)
{
	m_localAngle.y = localAngleY;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
}

void GameObject::ApplyLocalAngleZ(float localAngleZ)
{
	m_localAngle.z = localAngleZ;
	MarkTransformDirtyRecursive();
	if (IsPhysicsWorldLocked())
	{
		QueueHierarchyPhysicsTransformSync();
	}
	else
	{
		SyncHierarchyPhysicsTransform();
	}
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
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();

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
    obj->ApplyWorldPosition(data.position);
    obj->ApplyWorldSize(data.size);
    obj->ApplyWorldAngleX(data.angle.x);
    obj->ApplyWorldAngleY(data.angle.y);
    obj->ApplyWorldAngleZ(data.angle.z);
    return obj;
}

Component* GameObject::AddComponent(Component* component) {
    return AddComponent(component, true, true);
}

Component* GameObject::AddComponent(Component* component, bool initializeComponent, bool startComponent) {
    if (component) {
        component->InitGameObj(this);
        m_vecComponent->push_back(component);
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

void GameObject::FlushPendingStateChanges()
{
    if (!m_hasPendingPhysicsActiveStateChange || IsPhysicsWorldLocked())
    {
        FlushPendingTransformSync();
        return;
    }

    m_hasPendingPhysicsActiveStateChange = false;
    ApplyPhysicsActiveState(m_pendingPhysicsActiveState);
    FlushPendingTransformSync();
}

void GameObject::ApplyPhysicsActiveState(bool isActive)
{
    if (m_vecComponent == nullptr)
    {
        return;
    }

    vector<Collider2D*> colliders;
    colliders.reserve(m_vecComponent->size());

    for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++)
    {
        Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
        if (collider != nullptr)
        {
            colliders.push_back(collider);
        }
    }

    MainFrame* mainFrame = MainFrame::GetInstance();
    if (!isActive && mainFrame != nullptr)
    {
        for (vector<Collider2D*>::iterator itr = colliders.begin(); itr != colliders.end(); itr++)
        {
            mainFrame->ResetCollisionPairsForCollider(*itr);
            mainFrame->ResetTriggerPairsForCollider(*itr);
        }
    }

    Rigidbody2D* rigidbody2D = GetComponent<Rigidbody2D>();
    if (rigidbody2D != nullptr)
    {
        rigidbody2D->SetPhysicsActive(isActive);
    }

    for (vector<Collider2D*>::iterator itr = colliders.begin(); itr != colliders.end(); itr++)
    {
        Collider2D* collider = *itr;
        if (collider == nullptr)
        {
            continue;
        }

        collider->SetPhysicsActive(isActive);
    }
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
    SyncExternalSizeWriteback();
    FlushPendingStateChanges();
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

    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    const D3DXVECTOR3 worldPosition = m_position;
    const D3DXVECTOR3 worldSize = m_size;
    const float worldAngleX = m_angleX;
    const float worldAngleY = m_angleY;
    const float worldAngleZ = m_angleZ;

    if (m_parent != nullptr)
    {
        m_parent->DeleteChild(this);
    }

    m_parent = obj;
    if (obj != nullptr)
    {
        obj->m_children->push_back(this);
    }

    ApplyWorldSize(worldSize);
    ApplyWorldAngleX(worldAngleX);
    ApplyWorldAngleY(worldAngleY);
    ApplyWorldAngleZ(worldAngleZ);
    ApplyWorldPosition(worldPosition);
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

    obj->SetParent(this);
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
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    return m_angleZ;
}

const float& GameObject::GetAngleX()
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    return m_angleX;
}

const float& GameObject::GetAngleY()
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    return m_angleY;
}

void GameObject::SetAngleZ(float v)
{
    SyncExternalSizeWriteback();
    ApplyWorldAngleZ(v);
}

void GameObject::SetAngleX(float v)
{
    SyncExternalSizeWriteback();
    ApplyWorldAngleX(v);
}

void GameObject::SetAngleY(float v)
{
    SyncExternalSizeWriteback();
    ApplyWorldAngleY(v);
}

const D3DXMATRIX& GameObject::GetWorldMatrix()
{
    SyncExternalSizeWriteback();
    EnsureTransformCurrent();
    return m_worldMatrix;
}

void GameObject::OnCollisionEnter(Collider2D* col)
{
    if (m_vecComponent == nullptr || m_isDestroy)
        return;

    DispatchComponentEventSnapshot(this, [col](Component* component)
    {
        component->OnCollisionEnter(col);
    }, true);
}

void GameObject::OnCollisionStay(Collider2D* col)
{
	if (m_vecComponent == nullptr || m_isDestroy)
        return;

	DispatchComponentEventSnapshot(this, [col](Component* component)
	{
		component->OnCollisionStay(col);
	}, true);
}

void GameObject::OnCollisionExit(Collider2D* col)
{
	if (m_vecComponent == nullptr)
        return;

	DispatchComponentEventSnapshot(this, [col](Component* component)
	{
		component->OnCollisionExit(col);
	}, false);
}

void GameObject::OnTriggerEnter(Collider2D* col)
{
    if (m_vecComponent == nullptr || m_isDestroy)
        return;

    DispatchComponentEventSnapshot(this, [col](Component* component)
    {
        component->OnTriggerEnter(col);
    }, true);
}

void GameObject::OnTriggerStay(Collider2D* col)
{
    if (m_vecComponent == nullptr || m_isDestroy)
        return;

    DispatchComponentEventSnapshot(this, [col](Component* component)
    {
        component->OnTriggerStay(col);
    }, true);
}

void GameObject::OnTriggerExit(Collider2D* col)
{
    if (m_vecComponent == nullptr)
        return;

    DispatchComponentEventSnapshot(this, [col](Component* component)
    {
        component->OnTriggerExit(col);
    }, false);
}

void GameObject::OnLBtnDown()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnLBtnDown();
    }, true);
}

void GameObject::OnLBtnUp()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnLBtnUp();
    }, true);
}

void GameObject::OnRBtnDown()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnRBtnDown();
    }, true);
}

void GameObject::OnRBtnUp()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnRBtnUp();
    }, true);
}

void GameObject::OnMouseHoverEnter()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnMouseHoverEnter();
    }, true);
}

void GameObject::OnMouseHoverStay()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnMouseHoverStay();
    }, true);
}

void GameObject::OnMouseHoverExit()
{
    if (!CanDispatchMouseEvent(this))
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnMouseHoverExit();
    }, true);
}

void GameObject::OnEnable()
{
    if (m_vecComponent == nullptr || m_isDestroy || !m_setActive)
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnEnable();
    }, true);
}

void GameObject::OnDisable()
{
    if (m_vecComponent == nullptr || m_isDestroy || m_setActive)
        return;

    DispatchComponentEventSnapshot(this, [](Component* component)
    {
        component->OnDisable();
    }, true);
}


