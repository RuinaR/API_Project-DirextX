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
	// 충돌 Exit는 상대 오브젝트가 이미 삭제 예약된 경우 nullptr로 들어올 수 있다.
	// 이는 삭제 등으로 contact가 강제/불완전 종료된 상황을 의미한다.
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
	// 사용자 컴포넌트는 이벤트 중 DestroyObject를 호출할 수 있다.
	// 단, 즉시 삭제가 아니라 예약 삭제이며 Exit의 other는 nullptr일 수 있다.
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
