#pragma once
#include "Component.h"
#include "structs.h"
#include "BoxCollider.h"
class GameObject
{
protected:
    bool m_isDestroy = false;
    bool m_setActive = true;
    string m_tag = "";
    D3DXVECTOR3 m_position = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 m_size = { 0.0f, 0.0f, 0.0f };
    float m_angleZ;
    float m_angleX;
    float m_angleY;

    vector<Component*>* m_vecComponent = nullptr;
    GameObject* m_parent = nullptr;
    vector<GameObject*>* m_children;
    BoxCollider* m_box;
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
        for (vector<Component*>::iterator itr = m_vecComponent->begin(); itr != m_vecComponent->end(); itr++) 
        {
            if (typeid(**itr) == typeid(T)) 
            {
                return (T*)*itr;
            }
        }
        return nullptr;
    }

    void SetDestroy(bool destroy);
    bool GetDestroy();
    void SetTag(string tag);
    string GetTag();
    void SetActive(bool isActive);
    bool GetActive();
    Component* AddComponent(Component* component);
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


    void OnCollisionEnter(Collider* col);
    void OnCollisionStay(Collider* col);
    void OnCollisionExit(Collider* col);

    void OnLBtnDown();
    void OnLBtnUp();
    void OnRBtnDown();
    void OnRBtnUp();
};
