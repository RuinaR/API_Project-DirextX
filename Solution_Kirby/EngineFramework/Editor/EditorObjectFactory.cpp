#include "pch.h"
#include "EditorObjectFactory.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "ImageRender.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UILabel.h"

std::string EditorObjectFactory::CreateUniqueGameObjectName(const std::string& baseName)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr || !objectManager->HasGameObjectName(baseName))
	{
		return baseName;
	}

	int index = 1;
	while (true)
	{
		std::ostringstream oss;
		oss << baseName << " (" << index << ")";
		const std::string candidate = oss.str();
		if (!objectManager->HasGameObjectName(candidate))
		{
			return candidate;
		}
		index++;
	}
}

GameObject* EditorObjectFactory::CreateEmptyGameObject()
{
	GameObject* obj = new GameObject();
	obj->SetTag(CreateUniqueGameObjectName("New GameObject"));
	obj->InitializeSet();
	return obj;
}

GameObject* EditorObjectFactory::CreateSpriteGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("Sprite"));
	ImageRender* imageRender = new ImageRender(nullptr);
	Component* addedComponent = obj->AddComponent(imageRender);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* EditorObjectFactory::CreateUIImageGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Image"));
	UIImage* image = new UIImage();
	Component* addedComponent = obj->AddComponent(image);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* EditorObjectFactory::CreateUIButtonGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Button"));
	UIButton* button = new UIButton();
	Component* addedComponent = obj->AddComponent(button);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

GameObject* EditorObjectFactory::CreateUITextGameObject()
{
	GameObject* obj = CreateEmptyGameObject();
	if (obj == nullptr)
	{
		return nullptr;
	}

	obj->SetTag(CreateUniqueGameObjectName("UI Text"));
	UILabel* label = new UILabel();
	Component* addedComponent = obj->AddComponent(label);
	ApplyDefaultComponentValues(obj, addedComponent);
	return obj;
}

void EditorObjectFactory::ApplyDefaultComponentValues(GameObject* obj, Component* component)
{
	if (obj == nullptr || component == nullptr)
	{
		return;
	}

	if (UIButton* button = dynamic_cast<UIButton*>(component))
	{
		const D3DXVECTOR2 size(180.0f, 48.0f);
		button->SetSize(&size);
		button->SetUseTexture(false);
		button->SetStateColors(
			D3DCOLOR_ARGB(255, 80, 120, 220),
			D3DCOLOR_ARGB(255, 100, 145, 240),
			D3DCOLOR_ARGB(255, 55, 90, 180));
		button->SetActionKey("");
		return;
	}

	if (UIImage* image = dynamic_cast<UIImage*>(component))
	{
		const D3DXVECTOR2 size(128.0f, 128.0f);
		image->SetSize(&size);
		image->SetUseTexture(false);
		image->SetColor(D3DCOLOR_ARGB(255, 80, 180, 220));
		return;
	}

	if (UILabel* label = dynamic_cast<UILabel*>(component))
	{
		const D3DXVECTOR2 size(240.0f, 48.0f);
		label->SetSize(&size);
		label->SetText(L"Text");
		label->SetFontSize(24);
		label->SetColor(D3DCOLOR_ARGB(255, 255, 255, 255));
		return;
	}

	if (ImageRender* imageRender = dynamic_cast<ImageRender*>(component))
	{
		obj->Size3D() = D3DXVECTOR3(128.0f, 128.0f, 1.0f);
		imageRender->SetUseTexture(false);
		imageRender->SetColor(D3DCOLOR_ARGB(255, 255, 255, 255));
		imageRender->SetRenderEnabled(true);
		return;
	}
}
