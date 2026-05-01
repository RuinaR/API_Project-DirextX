#include "pch.h"
#include "Physics2D/Physics2D.h"
#include <cfloat>
#include "Component/Collision/BoxCollider2D.h"
#include "Component/Collision/CircleCollider2D.h"
#include "GameObject.h"
#include "ObjectManager.h"

namespace
{
	D3DXVECTOR3 RotateVector2D(const D3DXVECTOR3& vector, float angle)
	{
		const float cosAngle = cosf(angle);
		const float sinAngle = sinf(angle);
		return D3DXVECTOR3(
			(vector.x * cosAngle) - (vector.y * sinAngle),
			(vector.x * sinAngle) + (vector.y * cosAngle),
			vector.z);
	}

	D3DXVECTOR3 CalculateBoxCenter(GameObject* obj, BoxCollider2D* boxCollider)
	{
		D3DXVECTOR3 center = obj != nullptr ? obj->Position() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		if (boxCollider == nullptr)
		{
			return center;
		}

		if (boxCollider->GetBody() != nullptr)
		{
			const b2Vec2 bodyPosition = boxCollider->GetBody()->GetPosition();
			center.x = bodyPosition.x;
			center.y = bodyPosition.y;
			if (!boxCollider->OwnsBody())
			{
				const D3DXVECTOR3 rotatedOffset = RotateVector2D(
					D3DXVECTOR3(boxCollider->GetColOffset().x, boxCollider->GetColOffset().y, 0.0f),
					boxCollider->GetBody()->GetAngle());
				center.x += rotatedOffset.x;
				center.y += rotatedOffset.y;
			}
		}
		else
		{
			const Vector2D& offset = boxCollider->GetColOffset();
			center.x += offset.x;
			center.y += offset.y;
		}

		return center;
	}

	float CalculateBoxAngle(BoxCollider2D* boxCollider)
	{
		if (boxCollider == nullptr || boxCollider->GetBody() == nullptr)
		{
			return 0.0f;
		}

		return boxCollider->GetBody()->GetAngle();
	}

	bool TryHitBoxCollider(
		GameObject* obj,
		BoxCollider2D* boxCollider,
		const D3DXVECTOR3& planeHitPoint,
		float planeHitDistance,
		bool includeTriggers,
		RaycastHit2D& outHit,
		bool& hasHit,
		float& closestDistance)
	{
		if (boxCollider == nullptr)
		{
			return false;
		}

		if (!includeTriggers && boxCollider->GetTrigger())
		{
			return false;
		}

		const D3DXVECTOR3 center = CalculateBoxCenter(obj, boxCollider);
		const float angle = CalculateBoxAngle(boxCollider);
		const Vector2D& size = boxCollider->GetColSize();
		const float halfWidth = size.x * 0.5f;
		const float halfHeight = size.y * 0.5f;

		D3DXVECTOR3 localPoint = planeHitPoint - center;
		localPoint = RotateVector2D(localPoint, -angle);

		if (fabs(localPoint.x) > halfWidth || fabs(localPoint.y) > halfHeight)
		{
			return false;
		}

		if (hasHit && planeHitDistance >= closestDistance)
		{
			return true;
		}

		const float leftDistance = fabs(localPoint.x + halfWidth);
		const float rightDistance = fabs(halfWidth - localPoint.x);
		const float bottomDistance = fabs(localPoint.y + halfHeight);
		const float topDistance = fabs(halfHeight - localPoint.y);

		float minDistance = leftDistance;
		D3DXVECTOR3 localNormal(-1.0f, 0.0f, 0.0f);

		if (rightDistance < minDistance)
		{
			minDistance = rightDistance;
			localNormal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		}
		if (bottomDistance < minDistance)
		{
			minDistance = bottomDistance;
			localNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		}
		if (topDistance < minDistance)
		{
			localNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		}

		hasHit = true;
		closestDistance = planeHitDistance;
		outHit.collider = boxCollider;
		outHit.gameObject = obj;
		outHit.point = planeHitPoint;
		outHit.normal = RotateVector2D(localNormal, angle);
		outHit.distance = planeHitDistance;
		outHit.isTrigger = boxCollider->GetTrigger();
		return true;
	}

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

	D3DXVECTOR3 CalculateCircleCenter(GameObject* obj, CircleCollider2D* circleCollider)
	{
		D3DXVECTOR3 center = obj != nullptr ? obj->Position() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		if (circleCollider == nullptr)
		{
			return center;
		}

		const Vector2D& offset = circleCollider->GetColOffset();
		if (circleCollider->GetBody() != nullptr)
		{
			const b2Vec2 bodyPosition = circleCollider->GetBody()->GetPosition();
			center.x = bodyPosition.x;
			center.y = bodyPosition.y;
			if (!circleCollider->OwnsBody())
			{
				center.x += offset.x;
				center.y += offset.y;
			}
		}
		else
		{
			center.x += offset.x;
			center.y += offset.y;
		}

		return center;
	}

	D3DXVECTOR3 CalculateCircleNormal(const D3DXVECTOR3& hitPoint, const D3DXVECTOR3& center)
	{
		D3DXVECTOR3 normal = hitPoint - center;
		const float lengthSq = (normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z);
		if (lengthSq <= 0.000001f)
		{
			return D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		}

		D3DXVec3Normalize(&normal, &normal);
		return normal;
	}
}

Collider2D* Physics2D::Raycast(const Ray& ray, float maxDistance, bool includeTriggers)
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

		vector<Component*>* components = obj->GetComponentVec();
		if (components == nullptr)
		{
			continue;
		}

		for (vector<Component*>::iterator componentItr = components->begin(); componentItr != components->end(); componentItr++)
		{
			Component* component = *componentItr;
			if (component == nullptr)
			{
				continue;
			}

			BoxCollider2D* boxCollider = dynamic_cast<BoxCollider2D*>(component);
			if (boxCollider != nullptr)
			{
				TryHitBoxCollider(obj, boxCollider, planeHitPoint, planeHitDistance, includeTriggers, outHit, hasHit, closestDistance);
				continue;
			}

			CircleCollider2D* circleCollider = dynamic_cast<CircleCollider2D*>(component);
			if (circleCollider == nullptr)
			{
				continue;
			}

			if (!includeTriggers && circleCollider->GetTrigger())
			{
				continue;
			}

			const D3DXVECTOR3 center = CalculateCircleCenter(obj, circleCollider);
			const float radius = circleCollider->GetRadius();
			const float dx = planeHitPoint.x - center.x;
			const float dy = planeHitPoint.y - center.y;
			const float radiusSq = radius * radius;
			const float distanceSq = (dx * dx) + (dy * dy);

			if (distanceSq <= radiusSq)
			{
				if (!hasHit || planeHitDistance < closestDistance)
				{
					hasHit = true;
					closestDistance = planeHitDistance;
					outHit.collider = circleCollider;
					outHit.gameObject = obj;
					outHit.point = planeHitPoint;
					outHit.normal = CalculateCircleNormal(planeHitPoint, center);
					outHit.distance = planeHitDistance;
					outHit.isTrigger = circleCollider->GetTrigger();
				}
			}
		}
	}

	return hasHit;
}
