#pragma once
#include "Math/Ray.h"

class Collider;
class GameObject;

struct RaycastHit2D
{
	Collider* collider = nullptr;
	GameObject* gameObject = nullptr;
	D3DXVECTOR3 point = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float distance = 0.0f;
	bool isTrigger = false;
};

namespace Physics2D
{
	Collider* Raycast(const Ray& ray, float maxDistance, bool includeTriggers = false);
	bool Raycast(const Ray& ray, RaycastHit2D& outHit, float maxDistance, bool includeTriggers = false);
}
