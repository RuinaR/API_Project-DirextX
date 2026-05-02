#include "pch.h"
#include "GameActionRegistry.h"
#include "UIActionRegistry.h"
#include "Scripts/ActivePhysicsTestActions.h"
#include "Scripts/SampleButtonAction.h"

void RegisterGameUIActions(RenderType type)
{
	(void)type;

	// 사용자 UI action 등록 지점:
	// - 실제 action callback은 KirbyGameDll/UserActions/Scripts에 둔다.
	// - UIButton은 SceneData에 actionKey만 저장하고, startup 시 registry에서 다시 bind 된다.
	// - 새 C++ action 추가 후에는 DLL 재빌드 + 에디터 재실행이 필요하다.
	// - 에디터 실행 중 C++ action hot reload는 지원하지 않는다.
	UIActionRegistry::RegisterAction(
		ActivePhysicsTestActionKeys::kToggleActiveActionKey,
		[]() { ExecuteActivePhysicsTestTogglePlayerActiveAction(); });
	UIActionRegistry::RegisterAction(
		ActivePhysicsTestActionKeys::kToggleTriggerActionKey,
		[]() { ExecuteActivePhysicsTestTogglePlayerTriggerAction(); });
	UIActionRegistry::RegisterAction("SampleButtonAction", []() { ExecuteSampleButtonAction(); });
}
