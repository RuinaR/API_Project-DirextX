#pragma once

#include "GameObject.h"
#include "ObjectManager.h"

class EditorSelectionService
{
public:
	static GameObject* GetSelectedObject()
	{
		return GetSelectedObjectStorage();
	}

	static void SetSelectedObject(GameObject* obj)
	{
		GetSelectedObjectStorage() = obj;
	}

	static void ClearSelectedObject()
	{
		GetSelectedObjectStorage() = nullptr;
	}

	static void EnsureObjectManagerHooksRegistered()
	{
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (objectManager == nullptr)
		{
			return;
		}

		ObjectManager*& registeredManager = GetRegisteredManagerStorage();
		if (registeredManager == objectManager)
		{
			return;
		}

		objectManager->RegisterOnObjectDestroyRequested(&EditorSelectionService::HandleObjectDestroyRequested);
		objectManager->RegisterOnObjectReleased(&EditorSelectionService::HandleObjectReleased);
		objectManager->RegisterOnObjectsClearing(&EditorSelectionService::HandleObjectsClearing);
		objectManager->RegisterOnObjectsCleared(&EditorSelectionService::HandleObjectsCleared);
		registeredManager = objectManager;
	}

private:
	static GameObject*& GetSelectedObjectStorage()
	{
		static GameObject* selectedObject = nullptr;
		return selectedObject;
	}

	static ObjectManager*& GetRegisteredManagerStorage()
	{
		static ObjectManager* registeredManager = nullptr;
		return registeredManager;
	}

	static void HandleObjectDestroyRequested(GameObject* obj)
	{
		if (obj != nullptr && obj->IsSameOrChild(GetSelectedObject()))
		{
			ClearSelectedObject();
		}
	}

	static void HandleObjectReleased(GameObject* obj)
	{
		if (GetSelectedObject() == obj)
		{
			ClearSelectedObject();
		}
	}

	static void HandleObjectsClearing()
	{
		ClearSelectedObject();
	}

	static void HandleObjectsCleared()
	{
		ClearSelectedObject();
	}
};
