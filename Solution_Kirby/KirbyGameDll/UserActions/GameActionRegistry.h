#pragma once

enum class RenderType;

// KirbyGameDll의 사용자 UI action 등록 진입점이다.
// UIButton 등은 SceneData에 actionKey만 저장하고,
// 실제 callback 연결은 DLL 재빌드 후 여기서 다시 등록한다.
// 작성 절차:
// 1) 실제 action callback은 KirbyGameDll/UserActions/Scripts에 작성한다.
// 2) 여기서 UIActionRegistry::RegisterAction("ActionKey", callback)으로 등록한다.
// 3) UIButton Inspector에서 actionKey를 선택한다.
// 4) SceneData에는 callback/lambda 대신 actionKey 문자열만 저장된다.
// 5) 새 C++ action 추가 후에는 DLL 재빌드 + 에디터 재실행이 필요하다.
// 6) 에디터 실행 중 C++ action hot reload는 지원하지 않는다.
// 7) EngineFramework에는 게임별 action callback 코드를 넣지 않는다.
void RegisterGameUIActions(RenderType type);
