#pragma once
#include <iostream>
#include <unordered_map>
class GameObject;
class Collider;

class Component abstract
{
protected:
	GameObject* m_gameObj = nullptr;

	virtual void CollisionEnter(Collider* other) {};
	virtual void CollisionExit(Collider* other) {};
	virtual void CollisionStay(Collider* other) { Collision(other); }
	virtual void Collision(Collider* other) {};

public:
	Component();
	virtual ~Component();
	virtual void Initialize() abstract;
	virtual void Release() abstract;
	virtual void Start() abstract;
	virtual void Update() abstract;
	void OnCollisionEnter(Collider* other) { CollisionEnter(other); }
	void OnCollisionExit(Collider* other) { CollisionExit(other); }
	void OnCollisionStay(Collider* other) { CollisionStay(other); }
	void OnCollision(Collider* other) { OnCollisionStay(other); }
	virtual void InitGameObj(GameObject* obj);
	GameObject* GetGameObject();

	virtual void OnLBtnDown();
	virtual void OnLBtnUp();
	virtual void OnRBtnDown();
	virtual void OnRBtnUp();
	virtual void OnMouseHover();
	virtual const char* GetInspectorName() const;
	virtual void DrawInspector();
	virtual const char* GetSerializableType() const;
	virtual std::string Serialize() const;
	virtual bool Deserialize(const std::string& componentJson);
	virtual void ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap);
};
