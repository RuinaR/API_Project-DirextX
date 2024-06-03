#pragma once
#include <iostream>
class GameObject;
class Collider;

class Component abstract
{
protected:
	GameObject* m_gameObj = nullptr;

	virtual void CollisionEnter(Collider* other) {};
	virtual void CollisionExit(Collider* other) {};
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
	void OnCollision(Collider* other) { Collision(other); }
	void InitGameObj(GameObject* obj);
	GameObject* GetGameObject();

	virtual void OnLBtnDown();
	virtual void OnLBtnUp();
	virtual void OnRBtnDown();
	virtual void OnRBtnUp();
	virtual void OnMouseHover();
};

