#include "pch.h"
#include "GameActionRegistry.h"
#include "UIActionRegistry.h"
#include "Scripts/ActivePhysicsTestActions.h"
#include "Scripts/SampleButtonAction.h"

void RegisterGameUIActions(RenderType type)
{
	(void)type;

	// 사용자 UI 액션을 등록하는 곳이다.
	// - 실제 액션 함수는 KirbyGameDll/UserActions/Scripts 아래에 둔다.
	// - UIButton은 SceneData에 actionKey만 저장하고, 시작할 때 registry에서 다시 연결한다.
	// - 새 C++ 액션을 추가했다면 DLL을 다시 빌드하고 에디터도 다시 실행해야 한다.
	// - 에디터를 켠 상태에서 C++ 액션만 바로 바꿔 끼우는 기능은 아직 없다.
	UIActionRegistry::RegisterAction(
		ActivePhysicsTestActionKeys::kToggleActiveActionKey,
		[]() { ExecuteActivePhysicsTestTogglePlayerActiveAction(); });
	UIActionRegistry::RegisterAction(
		ActivePhysicsTestActionKeys::kToggleTriggerActionKey,
		[]() { ExecuteActivePhysicsTestTogglePlayerTriggerAction(); });
	UIActionRegistry::RegisterAction("SampleButtonAction", []() { ExecuteSampleButtonAction(); });
}
