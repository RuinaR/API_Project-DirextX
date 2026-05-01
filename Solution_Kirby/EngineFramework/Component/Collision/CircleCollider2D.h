#pragma once
#include "Collider2D.h"

class CircleCollider2D : public Collider2D
{
protected:
	void ColInit() final;
	void ColRelease() final;
	b2Fixture* CreateFixtureOnBody(b2Body* body) final;

public:
	CircleCollider2D(b2BodyType type);
	~CircleCollider2D() override;

	float GetRadius() const;
	void SetRadius(float radius);
	void DebugRenderUpdate() override;

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
