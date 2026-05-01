#pragma once
#include "Component.h"
#include "structs.h"

struct GameObjectSerializedData
{
    int objectId = -1;
    int parentId = -1;
    string name;
    string tag;
    bool active = true;
    D3DXVECTOR3 position = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 size = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 angle = { 0.0f, 0.0f, 0.0f };
};

class GameObject
{
protected:
    bool m_isDestroy = false;
    bool m_setActive = true;
    int m_objectId = -1;
    string m_tag = "";
    D3DXVECTOR3 m_position = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_size = { 0.0f, 0.0f, 0.0f };
    float m_angleZ;
    float m_angleX;
    float m_angleY;

    vector<Component*>* m_vecComponent = nullptr;
    vector<Component*>* m_pendingDeleteComponents = nullptr;
    GameObject* m_parent = nullptr;
    vector<GameObject*>* m_children;
    void FlushPendingComponents();
public:
    GameObject();
    virtual ~GameObject();

    const D3DXVECTOR3& Position();
    void SetPosition(D3DXVECTOR3 v);
	void AddPosition(D3DXVECTOR3 v);

    D3DXVECTOR2 Size2D();
    D3DXVECTOR3& Size3D();

    template <typename T>
    T* GetComponent() 
    {
        if (m_vecComponent == nullptr)
            return nullptr;

        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++) 
        {
            if (*itr == nullptr)
                continue;

            if (typeid(**itr) == typeid(T)) 
            {
                return (T*)*itr;
            }
        }
        return nullptr;
    }

    void SetDestroy(bool destroy);
    bool GetDestroy();
    int GetId() const;
    void SetId(int objectId);
    void SetTag(string tag);
    string GetTag();
    void SetActive(bool isActive);
    bool GetActive();
    std::string Serialize() const;
    std::string Serialize(int objectId, int parentId) const;
    std::string Serialize(int objectId, int parentId, int sceneVersion) const;
    static bool Deserialize(const std::string& objectJson, GameObjectSerializedData& outData);
    static bool Deserialize(const std::string& objectJson, GameObjectSerializedData& outData, int sceneVersion);
    static GameObject* CreateFromSerializedData(const GameObjectSerializedData& data);
    Component* AddComponent(Component* component);
    Component* AddComponent(Component* component, bool initializeComponent, bool startComponent);
    void DeleteComponent(Component* component);
    vector<Component*>* GetComponentVec();
    void InitializeSet();
    void Release();
    void Start();
    void Update();
    void SetParent(GameObject* obj);
    void AddChild(GameObject* obj);
    void DeleteChild(GameObject* obj);
    const float& GetAngleZ();
    const float& GetAngleX();
    const float& GetAngleY();

    void SetAngleZ(float v);
    void SetAngleX(float v);
    void SetAngleY(float v);

    GameObject* GetParent() { return m_parent; }
    vector<GameObject*>* GetChild() { return m_children; }


    void OnCollisionEnter(Collider2D* col);
    void OnCollisionStay(Collider2D* col);
    void OnCollisionExit(Collider2D* col);
    void OnTriggerEnter(Collider2D* col);
    void OnTriggerStay(Collider2D* col);
    void OnTriggerExit(Collider2D* col);

    void OnLBtnDown();
    void OnLBtnUp();
    void OnRBtnDown();
    void OnRBtnUp();
    void OnMouseHoverEnter();
    void OnMouseHoverStay();
    void OnMouseHoverExit();
};
