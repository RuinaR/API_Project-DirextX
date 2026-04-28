#pragma once
#include "Collider.h"
class BoxCollider : public Collider
{
protected:
	void ColInit() final;
	void ColRelease() final;
	//void Update() final;
public:
	BoxCollider(b2BodyType type);
	~BoxCollider() override;
};

