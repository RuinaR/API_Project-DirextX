#pragma once
#include "Collider2D.h"
class BoxCollider2D : public Collider2D
{
protected:
	void ColInit() final;
	void ColRelease() final;
	b2Fixture* CreateFixtureOnBody(b2Body* body) final;
	// BoxCollider2D는 base Collider2D의 사각형 debug render 기본 구현을 그대로 사용한다.
	//void Update() final;
public:
	BoxCollider2D(b2BodyType type);
	~BoxCollider2D() override;
	const char* GetInspectorName() const override;
	const char* GetSerializableType() const override;
};

