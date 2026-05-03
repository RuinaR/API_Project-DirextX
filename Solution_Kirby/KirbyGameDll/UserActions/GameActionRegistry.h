#pragma once

enum class RenderType;

// KirbyGameDll 쪽 사용자 UI 액션 등록 진입점이다.
// UIButton은 SceneData에 actionKey만 저장하고,
// 실제 액션 연결은 DLL이 올라올 때 다시 등록한다.
// 작업 순서:
// 1) 실제 액션 함수는 KirbyGameDll/UserActions/Scripts에 만든다.
// 2) 여기서 UIActionRegistry::RegisterAction("ActionKey", callback)으로 등록한다.
// 3) UIButton 인스펙터에서 actionKey를 고른다.
// 4) SceneData에는 callback이나 lambda 대신 actionKey 문자열만 저장된다.
// 5) 새 C++ 액션을 추가했다면 DLL을 다시 빌드하고 에디터도 다시 실행해야 한다.
// 6) 에디터 실행 중 C++ 액션을 바로 다시 읽는 기능은 아직 없다.
// 7) EngineFrameworkDll에는 게임 전용 action 코드를 넣지 않는다.
void RegisterGameUIActions(RenderType type);
