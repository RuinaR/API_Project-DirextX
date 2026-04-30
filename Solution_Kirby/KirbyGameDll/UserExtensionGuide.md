# User Extension Guide

## User Component Checklist

- `KirbyGameDll/UserComponents/Scripts` 아래에 새 `.h/.cpp`를 만든다.
- `Component`를 상속한다.
- `static constexpr const char* kComponentType`를 작성한다.
- `GetSerializableType()`이 `kComponentType`을 반환하게 한다.
- `DrawInspector()`에 편집 UI를 구현한다.
- `Serialize()/Deserialize()`에 저장 가능한 값만 구현한다.
- `UserComponents/GameComponentRegistry.cpp`에서 `REGISTER_GAME_COMPONENT(...)`로 등록한다.
- DLL을 재빌드하고 에디터를 다시 실행한다.

## User Action Checklist

- `KirbyGameDll/UserActions/Scripts` 아래에 callback 코드를 작성한다.
- `UserActions/GameActionRegistry.cpp`에서 `UIActionRegistry::RegisterAction("ActionKey", callback)`으로 등록한다.
- 에디터의 `UIButton` Inspector에서 `actionKey`를 선택한다.
- SceneData에는 callback/lambda 대신 `actionKey` 문자열만 저장된다.
- DLL을 재빌드하고 에디터를 다시 실행한다.

## Policy

- C++ `Component/Action` hot reload는 지원하지 않는다.
- 새 C++ 코드 추가 후에는 `DLL 재빌드 + 에디터 재실행`이 필요하다.
- 게임별 `Component/Action` 코드는 `KirbyGameDll`에 둔다.
- `EngineFramework`는 공용 엔진 기능만 유지하고, 게임별 코드는 넣지 않는다.
