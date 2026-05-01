# User Extension Guide

## User Component Checklist

- `KirbyGameDll/UserComponents/Scripts` 아래에 새 `.h/.cpp` 파일을 만든다.
- `Component`를 상속한다.
- `static constexpr const char* kComponentType`를 선언한다.
- `GetSerializableType()`은 `kComponentType`을 반환하게 한다.
- `DrawInspector()`에서 필요한 Inspector UI를 구현한다.
- `Serialize()/Deserialize()`는 필요한 값만 구현한다.
- `UserComponents/GameComponentRegistry.cpp`에서 `REGISTER_GAME_COMPONENT(...)`로 등록한다.
- DLL을 다시 빌드하고 에디터를 다시 실행한다.

## User Action Checklist

- `KirbyGameDll/UserActions/Scripts` 아래에 callback 코드를 작성한다.
- `UserActions/GameActionRegistry.cpp`에서 `UIActionRegistry::RegisterAction("ActionKey", callback)`로 등록한다.
- 에디터의 `UIButton` Inspector에서 `actionKey`를 선택한다.
- SceneData에는 callback/lambda 대신 `actionKey` 문자열만 저장된다.
- DLL을 다시 빌드하고 에디터를 다시 실행한다.

## Collision / Trigger Policy

- 충돌 이벤트 인자 타입은 `Collider2D*` 기준이다.
- `CollisionEnter/Stay/Exit(Collider2D* other)`는 둘 다 non-trigger일 때만 들어온다.
- `TriggerEnter/Stay/Exit(Collider2D* other)`는 둘 중 하나라도 trigger면 양쪽 모두에 들어온다.
- `OnCollisionExit(nullptr)`와 `OnTriggerExit(nullptr)`는 삭제 예약 또는 강제 종료 상황에서 들어올 수 있다.
- 이벤트 중 `DestroyObject()` 호출은 허용되지만 즉시 삭제는 아니고 pending remove 방식이다.

## Physics2D Notes

- `Rigidbody2D`가 있으면 `Collider2D`들은 그 body에 fixture로 붙는다.
- `Rigidbody2D`가 없으면 단일 `Collider2D`는 static fallback body를 직접 생성해 쓴다.
- `Rigidbody2D` 없이 collider 여러 개를 한 오브젝트에 붙이는 구조는 현재 비권장이다.
- `BoxCollider2D` SceneData canonical type은 `"BoxCollider2D"`다.
- 기존 `"BoxCollider"`는 legacy alias로 계속 로드 가능하다.

## Policy

- C++ `Component/Action` hot reload는 지원하지 않는다.
- 새 C++ 코드 추가 후에는 `DLL 빌드 + 에디터 재실행`이 필요하다.
- 게임별 `Component/Action` 코드는 `KirbyGameDll`에 둔다.
- `EngineFramework`는 공용 엔진 기능만 유지하고, 게임별 코드의 직접 의존은 넣지 않는다.
