# Physics2D Status

## Current Scope

- 물리 시스템은 현재 2D 기준으로 정리되어 있다.
- 주요 타입은 `Collider2D`, `BoxCollider2D`, `CircleCollider2D`, `Rigidbody2D`, `Physics2D`다.
- `BoxCollider2D`는 canonical SceneData type `"BoxCollider2D"`를 사용한다.
- 기존 `"BoxCollider"` 문자열은 legacy alias로 계속 로드 가능하다.

## Implemented Behavior

- `Rigidbody2D`가 있으면 collider들은 해당 `b2Body`에 fixture로 붙는다.
- `Rigidbody2D`가 없으면 단일 `Collider2D`는 static fallback body를 직접 생성해서 사용한다.
- `BoxCollider2D`와 `CircleCollider2D`는 shape별 fixture 생성 책임을 각각 가진다.
- `Physics2D::Raycast`는 `BoxCollider2D`와 `CircleCollider2D`를 모두 지원한다.
- `CircleCollider2D` debug render는 원형 outline으로 표시된다.

## Collision / Trigger Event Policy

- fixture `userData`에서 `Collider2D*`를 읽어 이벤트를 보낸다.
- 둘 다 non-trigger일 때만 `CollisionEnter/Stay/Exit`를 보낸다.
- 둘 중 하나라도 trigger면 양쪽 모두 `TriggerEnter/Stay/Exit`를 보낸다.
- `Trigger` 옵션 전환 시 기존 active collision/trigger pair는 강제 종료되고 `Exit`가 먼저 전달된다.
- `OnCollisionExit(nullptr)`와 `OnTriggerExit(nullptr)`는 삭제 예약 또는 강제 종료 상황에서 허용한다.

## Delete Safety

- `DestroyObject()`는 즉시 delete가 아니라 pending remove 방식이다.
- 실제 `Release/delete`는 `ObjectManager::FlushPendingObjects()`에서 처리된다.
- collision / trigger pair tracking이 있어 삭제 시 살아 있는 상대에 `Exit(nullptr)`를 보낼 수 있다.
- `Collider2D::Release()`는 active collision/trigger pair를 먼저 제거해 dangling pointer를 막는다.
- `GameObject`의 collision / trigger / mouse 이벤트 전파는 snapshot 기반이라 callback 중 component 삭제에 더 안전하다.

## Mouse / Raycast Notes

- delete-pending 오브젝트에는 mouse down/up/hover stay를 보내지 않는다.
- hover 대상이 삭제 예약되면 `HoverExit` 없이 조용히 정리한다.
- `Physics2D::Raycast`는 `includeTriggers == false`일 때 box/circle trigger collider를 모두 제외한다.

## Test Assets

- 테스트 씬:
  - `SceneData/Physics2DTestScene.json`
  - `SceneData/Physics2DInteractionTestScene.json`
- 테스트 컴포넌트:
  - `Physics2DKeyboardTestComponent`
- 테스트 씬은 static frame, dynamic/kinematic rigidbody, box/circle collider, trigger 반응, fallback collider, parent/child 케이스를 포함한다.

## Remaining Structural Limits

- `Rigidbody2D` 없이 collider 여러 개를 붙인 fallback 구조는 여전히 비권장이다.
- `CollisionManager.h/.cpp`는 deprecated 성격의 옛 코드가 남아 있다.
- 실제 빌드/실행 검증은 사용자 환경에서 별도로 확인해야 한다.

## Build Notes

- 최근 변경 범위가 넓어서 새 에러가 보이면 먼저 include 누락과 rename 누락을 의심한다.
- `MainFrame.cpp`는 active pair tracking 때문에 `<unordered_set>` include가 필요하다.
- `KirbyGameDll` 사용자 컴포넌트는 `Collider` 대신 `Collider2D` 시그니처를 사용해야 한다.
