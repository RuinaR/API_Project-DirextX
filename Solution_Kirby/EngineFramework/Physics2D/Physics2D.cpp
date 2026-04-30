#include "pch.h"
#include "Physics2D/Physics2D.h"
#include <cfloat>
#include "Component/Collision/BoxCollider.h"
#include "GameObject.h"
#include "ObjectManager.h"

namespace
{
	bool TryIntersectRayWithPhysicsPlane(const Ray& ray, D3DXVECTOR3& outPoint, float& outDistance)
	{
		const float epsilon = 0.000001f;

		if (fabs(ray.direction.z) <= epsilon)
		{
			if (fabs(ray.origin.z) > epsilon)
			{
				return false;
			}

			outPoint = ray.origin;
			outDistance = 0.0f;
			return true;
		}

		const float t = -ray.origin.z / ray.direction.z;
		if (t < 0.0f)
		{
			return false;
		}

		outPoint = ray.origin + (ray.direction * t);
		outDistance = t;
		return true;
	}

	D3DXVECTOR3 CalculateBoxNormal(const D3DXVECTOR3& point, float minX, float maxX, float minY, float maxY)
	{
		const float leftDistance = fabs(point.x - minX);
		const float rightDistance = fabs(maxX - point.x);
		const float bottomDistance = fabs(point.y - minY);
		const float topDistance = fabs(maxY - point.y);

		float minDistance = leftDistance;
		D3DXVECTOR3 normal(-1.0f, 0.0f, 0.0f);

		if (rightDistance < minDistance)
		{
			minDistance = rightDistance;
			normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		}
		if (bottomDistance < minDistance)
		{
			minDistance = bottomDistance;
			normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		}
		if (topDistance < minDistance)
		{
			normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		}

		return normal;
	}
}

Collider* Physics2D::Raycast(const Ray& ray, float maxDistance, bool includeTriggers)
{
	RaycastHit2D hit;
	if (!Raycast(ray, hit, maxDistance, includeTriggers))
	{
		return nullptr;
	}

	return hit.collider;
}

bool Physics2D::Raycast(const Ray& ray, RaycastHit2D& outHit, float maxDistance, bool includeTriggers)
{
	outHit = RaycastHit2D();

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr || objectManager->GetObjList() == nullptr)
	{
		return false;
	}

	D3DXVECTOR3 planeHitPoint;
	float planeHitDistance = 0.0f;
	if (!TryIntersectRayWithPhysicsPlane(ray, planeHitPoint, planeHitDistance))
	{
		return false;
	}

	if (maxDistance < 0.0f)
	{
		maxDistance = FLT_MAX;
	}

	if (planeHitDistance > maxDistance)
	{
		return false;
	}

	bool hasHit = false;
	float closestDistance = maxDistance;

	for (list<GameObject*>::iterator itr = objectManager->GetObjList()->begin(); itr != objectManager->GetObjList()->end(); ++itr)
	{
		GameObject* obj = *itr;
		if (obj == nullptr || obj->GetDestroy() || !obj->GetActive())
		{
			continue;
		}

		BoxCollider* boxCollider = obj->GetComponent<BoxCollider>();
		if (boxCollider == nullptr)
		{
			continue;
		}

		if (!includeTriggers && boxCollider->GetTrigger())
		{
			continue;
		}

		D3DXVECTOR3 center = obj->Position();
		if (boxCollider->GetBody() != nullptr)
		{
			const b2Vec2 bodyPosition = boxCollider->GetBody()->GetPosition();
			center.x = bodyPosition.x;
			center.y = bodyPosition.y;
		}
		else
		{
			const Vector2D& offset = boxCollider->GetColOffset();
			center.x += offset.x;
			center.y += offset.y;
		}

		const Vector2D& size = boxCollider->GetColSize();
		const float halfWidth = size.x * 0.5f;
		const float halfHeight = size.y * 0.5f;
		const float minX = center.x - halfWidth;
		const float maxX = center.x + halfWidth;
		const float minY = center.y - halfHeight;
		const float maxY = center.y + halfHeight;

		if (planeHitPoint.x < minX || planeHitPoint.x > maxX || planeHitPoint.y < minY || planeHitPoint.y > maxY)
		{
			continue;
		}

		if (!hasHit || planeHitDistance < closestDistance)
		{
			hasHit = true;
			closestDistance = planeHitDistance;
			outHit.collider = boxCollider;
			outHit.gameObject = obj;
			outHit.point = planeHitPoint;
			outHit.normal = CalculateBoxNormal(planeHitPoint, minX, maxX, minY, maxY);
			outHit.distance = planeHitDistance;
			outHit.isTrigger = boxCollider->GetTrigger();
		}
	}

	return hasHit;
}
