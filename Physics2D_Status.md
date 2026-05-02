# Physics2D Status

## 현재 범위

- 물리 시스템은 현재 2D 전용 기준으로 정리되어 있습니다.
- 주요 타입:
  - `Collider2D`
  - `BoxCollider2D`
  - `CircleCollider2D`
  - `Rigidbody2D`
  - `Physics2D`
- `BoxCollider2D`의 canonical SceneData type은 `"BoxCollider2D"`입니다.
- legacy `"BoxCollider"` alias는 구버전 SceneData 호환용으로 계속 로드 가능합니다.
- 현재 원본 `SceneData/`에는 exact legacy `"BoxCollider"` 사용이 남아 있지 않습니다.

## 현재 구현 상태

- `Rigidbody2D`가 있으면 collider는 해당 body의 fixture로 붙습니다.
- `Rigidbody2D`가 없으면 각 `Collider2D`가 static fallback body를 직접 생성합니다.
- `BoxCollider2D`, `CircleCollider2D`가 각자 shape/fixture 생성을 담당합니다.
- `Physics2D::Raycast`는 `BoxCollider2D`와 `CircleCollider2D`를 모두 지원합니다.
- `CircleCollider2D` debug render는 원형 outline으로 표시됩니다.

## Collision / Trigger 이벤트 정책

- 실제 충돌 이벤트는 Box2D `ContactListener` 기반입니다.
- `fixture userData`에서 `Collider2D*`를 읽어 `GameObject -> Component`로 fan-out 합니다.
- 둘 다 non-trigger일 때만 `CollisionEnter/Stay/Exit`가 들어옵니다.
- 둘 중 하나라도 trigger면 양쪽 모두 `TriggerEnter/Stay/Exit`가 들어옵니다.
- trigger 상태 전환 시 기존 active collision/trigger pair를 정리하고 필요한 `Exit`를 먼저 전달합니다.
- 삭제 예약 또는 강제 종료 상황에서는 `OnCollisionExit(nullptr)`, `OnTriggerExit(nullptr)`를 허용합니다.

## Delete 안전성

- `DestroyObject()`는 즉시 delete가 아니라 pending remove 방식입니다.
- 실제 `Release/delete`는 `ObjectManager::FlushPendingObjects()`에서 처리됩니다.
- collision / trigger pair tracking이 있어 삭제 중에도 `Exit(nullptr)` 누락을 줄이는 구조입니다.
- `Collider2D::Release()`는 active pair를 먼저 제거해 dangling pointer를 막습니다.
- `GameObject`의 collision / trigger / mouse fan-out은 snapshot 기반이라 callback 중 component 삭제에 비교적 안전합니다.

## Rigidbody2D / Collider2D 상호작용

- `Rigidbody2D`가 body owner입니다.
- body type, drag, angular drag, gravity, fixed rotation, velocity, collision detection, interpolate 등을 관리합니다.
- `Rigidbody2D`가 삭제되면 같은 오브젝트의 collider는 fallback body로 복귀합니다.
- collider만 삭제되면 fixture만 제거되고 `Rigidbody2D` body는 유지될 수 있습니다.
- `GameObject` transform 변경 시 physics world lock 여부에 따라 즉시 sync 또는 pending sync를 사용합니다.

## Raycast 메모

- `Mouse::ScreenPointToRay()`가 camera view + projection 기준 ray를 생성합니다.
- `Physics2D::Raycast`는 `ObjectManager`의 object list를 순회해 box/circle collider를 검사합니다.
- `includeTriggers == false`면 trigger collider는 raycast 대상에서 제외됩니다.
- box는 offset/rotation 반영 판정을 사용하고, circle은 중심/반지름 기반 판정을 사용합니다.

## 테스트 씬 / 테스트 컴포넌트

### 테스트 씬

- `SceneData/Physics2DTestScene.json`
- `SceneData/Physics2DInteractionTestScene.json`
- `SceneData/ActivePhysicsToggleTestScene.json`

### 테스트 컴포넌트

- `Physics2DKeyboardTestComponent`
- `ActivePhysicsTestStatusComponent`
- `ActivePhysicsTestInfoComponent`
- `CollisionVisualTestComponent`

### 역할

- `Physics2DTestScene`: 기본 body/collider/fallback/multi-collider 경로 검증
- `Physics2DInteractionTestScene`: dynamic/kinematic/trigger interaction 검증
- `ActivePhysicsToggleTestScene`: active / trigger / UI action 연동 검증

## 현재 제약

- `Rigidbody2D` 없이 collider 여러 개를 붙이는 fallback 구조는 비권장입니다.
- `Physics2D::Raycast`는 collider type을 직접 `dynamic_cast`로 순회하는 구조라 shape 종류가 늘면 확장 비용이 있습니다.
- deprecated `CollisionManager.h/.cpp` 파일은 디스크에는 남아 있지만, 프로젝트 항목과 활성 충돌 경로에서는 제거되었습니다.

## 최근 정리 반영

- `CollisionManager` 프로젝트 항목 제거 완료
- `BoxCollider2D` canonical type 사용 상태 확인 완료
- legacy `"BoxCollider"` alias는 유지 중
- 원본 `SceneData/`와 `BuildOutput/SceneData/` 기준 exact `"BoxCollider"` 잔존 데이터는 확인되지 않음

## 후속 후보

- Runtime / Editor 분리 목표에 맞춘 physics/editor 의존성 정리
- collider shape 확장 시 raycast 경로 일반화
- fallback multi-collider 구조 장기 정책 결정
