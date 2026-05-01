#pragma once
#include "RenderManager.h"
class Collider2D : public Component, public DebugRender
{
protected:
	Vector2D m_colSize = { 0.f,0.f };
	Vector2D m_colOffset= { 0.f,0.f };

	virtual void ColInit() abstract;
	virtual void ColRelease() abstract;
	virtual b2Fixture* CreateFixtureOnBody(b2Body* body) = 0;
	// 기본 debug render는 box outline 기준이다.
	// box 이외의 shape은 DebugRenderUpdate()를 override해서 자체 윤곽을 그린다.
	void RenderCollider();

	bool m_isTrigger = false;

	b2Body* m_body = nullptr;
	b2Fixture* m_fixture = nullptr;
	// Rigidbody2D가 없으면 Collider는 자기 fallback body를 직접 생성/소유한다.
	// 단일 collider-only 오브젝트는 지원하지만, Rigidbody2D 없이 collider를 여러 개 붙이면
	// 각 collider가 독립 fallback body를 가져 transform sync 기준이 불안정해질 수 있다.
	// 여러 collider를 함께 사용할 때는 Rigidbody2D를 추가해 하나의 physics body를 공유하는 것을 권장한다.
	bool m_ownsBody = false;
	b2BodyType m_type;
public:
	
	Collider2D(b2BodyType type);
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
	// 기본 구현은 RenderCollider()를 호출하는 box outline 경로다.
	virtual void DebugRenderUpdate() override;
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
