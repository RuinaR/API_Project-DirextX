#include "pch.h"
#include "ActivePhysicsTestActions.h"

#include "Collider2D.h"
#include "GameObject.h"

namespace
{
	GameObject* g_activePhysicsTestPlayerObject = nullptr;

	Collider2D* FindFirstCollider(GameObject* gameObject)
	{
		if (gameObject == nullptr || gameObject->GetComponentVec() == nullptr)
		{
			return nullptr;
		}

		vector<Component*>* components = gameObject->GetComponentVec();
		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
			if (collider != nullptr)
			{
				return collider;
			}
		}

		return nullptr;
	}
}

void SetActivePhysicsTestPlayerObject(GameObject* playerObject)
{
	g_activePhysicsTestPlayerObject = playerObject;
}

void ExecuteActivePhysicsTestTogglePlayerActiveAction()
{
	GameObject* playerObject = g_activePhysicsTestPlayerObject;
	if (playerObject == nullptr)
	{
		OutputDebugStringA("ActivePhysicsTest: player object reference is missing for active toggle.\n");
		return;
	}

	playerObject->SetActive(!playerObject->GetActive());
}

void ExecuteActivePhysicsTestTogglePlayerTriggerAction()
{
	GameObject* playerObject = g_activePhysicsTestPlayerObject;
	if (playerObject == nullptr)
	{
		OutputDebugStringA("ActivePhysicsTest: player object reference is missing for trigger toggle.\n");
		return;
	}

	Collider2D* collider = FindFirstCollider(playerObject);
	if (collider == nullptr)
	{
		OutputDebugStringA("ActivePhysicsTest: collider not found for trigger toggle.\n");
		return;
	}

	collider->SetTrigger(!collider->GetTrigger());
}
