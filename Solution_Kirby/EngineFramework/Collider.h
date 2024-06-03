#pragma once

class Collider : public Component
{
protected:
	Vector2D m_colSize = { 0.f,0.f };
	Vector2D m_colOffset= { 0.f,0.f };
	set<Collider*>* m_ColObjSet = nullptr;

	virtual void ColInit() abstract;
	virtual void ColRelease() abstract;

	bool m_isTrigger = false;
public:
	void Start() final;
	void Update() final;
	Vector2D& ColSize();
	Vector2D& ColOffset();
	set<Collider*>* SetCol();
	void Initialize() final;
	void Release() final;
	virtual void DrawCollider();
	void SetTrigger(bool b);
	bool GetTrigger();
};


