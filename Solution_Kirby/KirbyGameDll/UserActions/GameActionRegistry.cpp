#include "pch.h"
#include "GameActionRegistry.h"
#include "UIActionRegistry.h"
#include "ObjectManager.h"

#include "GameObject.h"
#include "ImageRender.h"



void RegisterGameUIActions(RenderType type)
{
	(void)type;

	// 사용자 UI action 등록 지점:
	// - 실제 action callback은 KirbyGameDll/UserActions에 둔다.
	// - UIButton은 SceneData에 actionKey만 저장하고, startup 시 registry에서 다시 bind 된다.
	// - 새 C++ action 추가 후에는 DLL 재빌드 + 에디터 재실행이 필요하다.
	// - 에디터 실행 중 C++ action hot reload는 지원하지 않는다.
	UIActionRegistry::RegisterAction("SampleButtonAction", []()
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
		});
}
