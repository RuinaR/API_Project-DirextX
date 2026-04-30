#pragma once
#include "RenderManager.h"
class Collider : public Component, public DebugRender
{
protected:
	Vector2D m_colSize = { 0.f,0.f };
	Vector2D m_colOffset= { 0.f,0.f };

	virtual void ColInit() abstract;
	virtual void ColRelease() abstract;
	void RenderCollider();

	bool m_isTrigger = false;

	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;
	bool m_ownsBody = false;
	b2BodyType m_type;
public:
	
	Collider(b2BodyType type);
	void Start() final;
	void Update() override;
	const Vector2D& GetColSize();
	const Vector2D& GetColOffset();
	void CreateBody(Vector2D offset, Vector2D size, bool fixedRotation);
	void SetBodyType(b2BodyType type);
	void SetColliderSize(Vector2D size);
	void SetColliderOffset(Vector2D offset);
	void SetFixedRotation(bool fixedRotation);
	bool GetFixedRotation() const;
	void Initialize() final;
	void Release() final;
	void SetTrigger(bool b);
	bool GetTrigger();
	b2Body* GetBody();
	void ClearBodyReferenceIfMatches(b2Body* body);
	bool OwnsBody() const;

	// DebugRender을(를) 통해 상속됨
	virtual void DebugRenderUpdate() override;
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
