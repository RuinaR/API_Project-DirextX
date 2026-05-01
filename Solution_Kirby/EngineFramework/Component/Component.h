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
	virtual void CollisionStay(Collider* other) {};
	virtual void LBtnDown() {}
	virtual void LBtnUp() {}
	virtual void RBtnDown() {}
	virtual void RBtnUp() {}
	virtual void MouseHoverEnter() {}
	virtual void MouseHoverExit() {}
	virtual void MouseHoverStay() {}

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
	virtual void InitGameObj(GameObject* obj);
	GameObject* GetGameObject();

	void OnLBtnDown() { LBtnDown(); }
	void OnLBtnUp() { LBtnUp(); }
	void OnRBtnDown() { RBtnDown(); }
	void OnRBtnUp() { RBtnUp(); }
	void OnMouseHoverEnter() { MouseHoverEnter(); }
	void OnMouseHoverExit() { MouseHoverExit(); }
	void OnMouseHoverStay() { MouseHoverStay(); }
	virtual const char* GetInspectorName() const;
	virtual void DrawInspector();
	virtual const char* GetSerializableType() const;
	virtual std::string Serialize() const;
	virtual bool Deserialize(const std::string& componentJson);
	virtual void ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap);
};
