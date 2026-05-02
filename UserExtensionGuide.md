# User Extension Guide

이 문서는 `KirbyGameDll` 기준으로 사용자 확장 코드와 테스트/샘플 정책을 정리한 가이드입니다.

## 기본 원칙

- 게임별 `Component`와 `Action` 코드는 `EngineFramework`가 아니라 `KirbyGameDll`에 둡니다.
- 런타임 포인터, DirectX 포인터, Box2D 포인터, callback 본체는 SceneData에 저장하지 않습니다.
- SceneData에는 문자열 key, 숫자 값, bool 값, object id 같은 직렬화 가능한 데이터만 저장합니다.
- 현재 사용자 C++ 확장은 hot reload를 지원하지 않습니다.
- 새 `Component`나 `Action`을 추가/수정하면 `KirbyGameDll.dll`을 다시 빌드하고 에디터를 재실행하는 것을 기본으로 합니다.

## User Component 체크리스트

- 파일 위치:
  - `KirbyGameDll/UserComponents/Scripts`
- `Component`를 상속합니다.
- `GetSerializableType()`을 구현합니다.
- Inspector 편집이 필요하면 `DrawInspector()`를 구현합니다.
- SceneData 저장/복원이 필요하면 `Serialize()`, `Deserialize()`를 구현합니다.
- object reference가 있으면 `ResolveReferences(...)`를 구현합니다.
- `KirbyGameDll/UserComponents/GameComponentRegistry.cpp`에 등록합니다.

## User Action 체크리스트

- 파일 위치:
  - `KirbyGameDll/UserActions/Scripts`
- `KirbyGameDll/UserActions/GameActionRegistry.cpp`에서 등록합니다.
- `UIButton`은 callback 본체가 아니라 `actionKey`만 SceneData에 저장합니다.
- 로드 시 `UIActionRegistry`에서 같은 key로 다시 바인딩됩니다.

## Collision / Trigger 정책

- 충돌 이벤트 인자는 `Collider2D*` 기준입니다.
- 둘 다 non-trigger일 때만 `CollisionEnter/Stay/Exit(Collider2D* other)`가 들어옵니다.
- 둘 중 하나라도 trigger면 `TriggerEnter/Stay/Exit(Collider2D* other)`가 들어옵니다.
- 삭제 예약 또는 강제 종료 경로에서는 `OnCollisionExit(nullptr)`, `OnTriggerExit(nullptr)`가 들어올 수 있습니다.
- 이벤트 중 `DestroyObject()`를 호출해도 즉시 삭제되지 않고 pending remove 경로로 처리됩니다.

## Physics2D 메모

- `Rigidbody2D`가 있으면 연결된 `Collider2D`는 그 body의 fixture로 붙습니다.
- `Rigidbody2D`가 없으면 `Collider2D`가 static fallback body를 직접 생성해 사용합니다.
- `Rigidbody2D` 없이 collider를 여러 개 붙이는 fallback 구조는 비권장입니다.
- box collider의 canonical SceneData type은 `"BoxCollider2D"`입니다.
- 기존 `"BoxCollider"`는 구버전 SceneData 호환을 위해 legacy alias로 계속 로드됩니다.

## 테스트 / 샘플 컴포넌트 정책

- 아래 컴포넌트들은 실제 게임 로직용이 아니라 테스트, 검증, 예시 제공 목적의 사용자 컴포넌트입니다.
- 등록은 유지해도, 그대로 게임 전용 시스템으로 간주하지는 않습니다.

### 샘플 유지 컴포넌트

- `SampleSpinComponent`
  - 일반적인 사용자 확장 예시용 샘플 컴포넌트

- `Physics2DKeyboardTestComponent`
  - Physics2D 조작/반응 검증용 샘플 컴포넌트
  - 사용 씬:
    - `SceneData/Physics2DTestScene.json`
    - `SceneData/ActivePhysicsToggleTestScene.json`
    - `SceneData/Physics2DInteractionTestScene.json`
  - 조작 키:
    - `A/D` 또는 `Left/Right`: 좌우 이동
    - `W/Up/Space`: 위 impulse
    - `Q/E`: 회전
    - `R`: 초기 위치/회전으로 리셋
  - `Use Velocity Mode`를 켜면 `SetVelocity()`와 `SetAngularVelocity()` 경로를 사용합니다.

- `CollisionVisualTestComponent`
  - 충돌, 마우스 클릭, hover 반응을 시각적으로 확인하기 위한 샘플 컴포넌트
  - 사용 씬:
    - `SceneData/Sample.json`
  - 실제 게임 로직보다 충돌/입력 이벤트 반응 예시와 수동 검증 목적에 가깝습니다.

### 임시 테스트 유지 컴포넌트

- `ActivePhysicsTestStatusComponent`
  - Active/Trigger 전환 테스트용 상태 추적 컴포넌트
  - 사용 씬:
    - `SceneData/ActivePhysicsToggleTestScene.json`

- `ActivePhysicsTestInfoComponent`
  - ActivePhysics 테스트 씬의 UI 정보 갱신용 컴포넌트
  - 사용 씬:
    - `SceneData/ActivePhysicsToggleTestScene.json`

## SceneData 씬 역할 구분

- 실제 게임 로직용 씬과 테스트/검증용 씬은 문서 기준으로 구분해 관리하는 것을 권장합니다.
- 아래 씬들은 현재 게임 콘텐츠 씬이라기보다 예시 또는 기능 검증 목적의 SceneData입니다.

### 예시 씬

- `SceneData/Sample.json`
  - 분류: 예시 씬
  - 대표 확인 기능:
    - 기본 Sprite / Animation / UI / FBX 배치 예시
    - `CollisionVisualTestComponent` 반응 예시
    - 물리 오브젝트와 렌더/UI 오브젝트가 함께 있는 기본 샘플 구성

### 검증 씬

- `SceneData/Physics2DTestScene.json`
  - 분류: Physics2D 검증 씬
  - 대표 확인 기능:
    - Dynamic / Kinematic / Static body 조합
    - Box / Circle collider 동작
    - fallback collider와 multi-collider warning 케이스
    - `Physics2DKeyboardTestComponent` 조작 검증

- `SceneData/ActivePhysicsToggleTestScene.json`
  - 분류: Active/Physics 전환 검증 씬
  - 대표 확인 기능:
    - Active 전환 시 충돌/트리거 상태 변화
    - Trigger 토글과 UI 버튼 액션 연동
    - `ActivePhysicsTestStatusComponent`, `ActivePhysicsTestInfoComponent` 검증

- `SceneData/Physics2DInteractionTestScene.json`
  - 분류: Physics2D interaction 검증 씬
  - 대표 확인 기능:
    - Dynamic / Kinematic / Trigger 상호작용
    - Box / Circle / Trigger 조합 반응
    - 부모-자식 오브젝트와 collider 조합 확인
    - `Physics2DKeyboardTestComponent` 기반 상호작용 검증

## 현재 정책 요약

- 게임 전용 `Component/Action` 코드는 `KirbyGameDll`에 둡니다.
- 사용자 확장 코드는 registry 등록이 빠지면 editor/SceneData와 연결되지 않습니다.
- SceneData에는 직렬화 가능한 데이터만 저장합니다.
- 테스트/샘플 컴포넌트는 실제 게임 로직과 분리해서 관리합니다.
