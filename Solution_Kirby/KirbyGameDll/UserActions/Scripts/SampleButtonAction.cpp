#include "pch.h"
#include "SampleButtonAction.h"

#include "GameObject.h"
#include "ImageRender.h"

void ExecuteSampleButtonAction()
{
	OutputDebugStringA("SampleButtonAction executed.\n");

	GameObject* spriteObject = new GameObject();
	spriteObject->SetTag("SampleSpawnedSprite");
	spriteObject->SetPosition({ 100.0f, 100.0f, 100.0f });
	spriteObject->Size3D() = { 128.0f, 128.0f, 1.0f };

	ImageRender* imageRender = new ImageRender(nullptr);
	imageRender->SetUseTexture(false);
	imageRender->SetColor(D3DCOLOR_ARGB(255, 80, 220, 120));
	imageRender->SetRenderEnabled(true);
	imageRender->SetTrans(true);
	spriteObject->AddComponent(imageRender);

	spriteObject->InitializeSet();
}
