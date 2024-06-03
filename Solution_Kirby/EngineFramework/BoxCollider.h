#pragma once
#include "Collider.h"
class BoxCollider : public Collider
{
protected:
	void ColInit() final;
	void ColRelease() final;
public:
	BoxCollider();
	~BoxCollider() override;
};

