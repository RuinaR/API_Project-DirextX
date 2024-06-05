#pragma once

class Collider : public Component
{
protected:
	Vector2D m_colSize = { 0.f,0.f };
	Vector2D m_colOffset= { 0.f,0.f };

	virtual void ColInit() abstract;
	virtual void ColRelease() abstract;

	bool m_isTrigger = false;

	b2Body* m_body;
	b2BodyType m_type;
public:
	Collider(b2BodyType type);
	void Start() final;
	void Update() final;
	const Vector2D GetColSize();
	void SetColSize(Vector2D size);
	Vector2D& ColOffset();
	void Initialize() final;
	void Release() final;
	void SetTrigger(bool b);
	bool GetTrigger();
	b2Body* GetBody();
};


