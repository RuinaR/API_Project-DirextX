# GameObject 이벤트 호출 시점 정리

이 문서는 현재 `Solution_Kirby` 코드 기준으로 `GameObject`와 각 `Component`에 전달되는 주요 이벤트의 호출 시점을 정리한 문서입니다.

대상 이벤트:

- `OnCollisionEnter(Collider2D* col)`
- `OnCollisionStay(Collider2D* col)`
- `OnCollisionExit(Collider2D* col)`
- `OnTriggerEnter(Collider2D* col)`
- `OnTriggerStay(Collider2D* col)`
- `OnTriggerExit(Collider2D* col)`
- `OnLBtnDown()`
- `OnLBtnUp()`
- `OnRBtnDown()`
- `OnRBtnUp()`
- `OnMouseHoverEnter()`
- `OnMouseHoverStay()`
- `OnMouseHoverExit()`
- `OnEnable()`
- `OnDisable()`

## 1. 공통 전달 방식

- 이벤트는 먼저 `GameObject`가 받고, 그 시점에 붙어 있는 `Component`들에게 순서대로 fan-out 됩니다.
- fan-out은 component 목록 snapshot 기반이라 callback 중 component 삭제가 발생해도 비교적 안전하게 순회됩니다.
- 일부 이벤트는 처리 중 `DestroyObject()`가 예약되면 이후 전달을 중단합니다.
- `OnCollisionExit`, `OnTriggerExit`는 종료 성격 이벤트라 destroy 예약 상황에서도 남은 component에 계속 전달될 수 있습니다.

## 2. Active 전환 이벤트

### `OnEnable()`

호출 시점:

- `GameObject::SetActive(true)` 호출 후
- 이전 상태가 `false`이고 새 상태가 `true`일 때만 호출

중요:

- 생성 직후 자동 호출 이벤트가 아닙니다.
- SceneData 로드 후 active 상태로 만들어진 오브젝트에도 자동 호출되지 않습니다.
- 즉, 현재 의미는 "생성 알림"이 아니라 "비활성 -> 활성 전환 알림"에 가깝습니다.

### `OnDisable()`

호출 시점:

- `GameObject::SetActive(false)` 호출 후
- 이전 상태가 `true`이고 새 상태가 `false`일 때만 호출

중요:

- inactive 전환 시 body / fixture도 함께 비활성화됩니다.
- 이 과정에서 기존 collision / trigger pair가 정리되며 필요한 `Exit` 이벤트가 먼저 들어올 수 있습니다.

## 3. Collision 이벤트

기반:

- Box2D `ContactListener`
- 둘 다 non-trigger일 때만 collision 계열 이벤트 사용

### `OnCollisionEnter(Collider2D* col)`

호출 시점:

- Box2D `BeginContact`
- 둘 다 non-trigger contact일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 inactive면 전달하지 않음
- receiver 또는 other 쪽 `GameObject`가 destroy pending이면 전달하지 않음

### `OnCollisionStay(Collider2D* col)`

호출 시점:

- Box2D `PreSolve`
- non-trigger contact가 유지 중일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 inactive면 전달하지 않음
- receiver 또는 other 쪽 `GameObject`가 destroy pending이면 전달하지 않음

### `OnCollisionExit(Collider2D* col)`

호출 시점:

- Box2D `EndContact`
- 또는 tracked collision pair 강제 정리 시점

인자 정책:

- 상대가 정상 상태면 상대 `Collider2D*`
- 상대가 이미 destroy pending 또는 destroy 상태면 `nullptr`

즉, `OnCollisionExit(nullptr)`는 정상적인 정리 케이스입니다.

## 4. Trigger 이벤트

기반:

- 둘 중 하나라도 trigger(sensor)면 trigger 계열 이벤트 사용

### `OnTriggerEnter(Collider2D* col)`

호출 시점:

- Box2D `BeginContact`
- 두 fixture 중 하나 이상이 trigger일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 inactive면 전달하지 않음
- receiver 또는 other 쪽 `GameObject`가 destroy pending이면 전달하지 않음

### `OnTriggerStay(Collider2D* col)`

호출 시점:

- 매 프레임 `DispatchActiveTriggerStayEvents()`에서
- active trigger pair로 추적 중인 접촉에 대해 호출

중요:

- `TriggerStay`는 Box2D `PreSolve`가 아니라 엔진 쪽 active pair 추적 루프에서 호출됩니다.

### `OnTriggerExit(Collider2D* col)`

호출 시점:

- Box2D `EndContact`
- 또는 tracked trigger pair 강제 정리 시점

인자 정책:

- 상대가 정상 상태면 상대 `Collider2D*`
- 상대가 이미 destroy pending 또는 destroy 상태면 `nullptr`

즉, `OnTriggerExit(nullptr)`도 정상적인 정리 케이스입니다.

## 5. 마우스 버튼 이벤트

기반:

- `WindowFrame::WndProc`
- `ObjectManager::OnLBtnDown / OnLBtnUp / OnRBtnDown / OnRBtnUp`
- 현재 hover 중인 오브젝트 기준 전달

공통 조건:

- 현재 마우스가 가리키는 오브젝트여야 함
- active 상태여야 함
- destroy 상태가 아니어야 함

### `OnLBtnDown()`

- `WM_LBUTTONDOWN` 수신 시

### `OnLBtnUp()`

- `WM_LBUTTONUP` 수신 시

### `OnRBtnDown()`

- `WM_RBUTTONDOWN` 수신 시

### `OnRBtnUp()`

- `WM_RBUTTONUP` 수신 시

중요:

- 클릭 처리 중 오브젝트가 destroy 예약되면 이후 hover target을 비워 후속 입력이 이어지지 않게 정리합니다.

## 6. Hover 이벤트

기반:

- `ObjectManager::UpdateMouseInteraction()`

호출 경로:

- 일반 게임/플레이 중에는 `ObjectManager::Update()` 안에서 매 프레임
- 에디터 `Paused` 상태에서도 hover/raycast 정보는 별도로 계속 갱신

### `OnMouseHoverEnter()`

- 이번 프레임 hover 대상이 이전 프레임과 다를 때
- 새 대상이 active이고 destroy 상태가 아닐 때

### `OnMouseHoverStay()`

- 이번 프레임에도 같은 hover 대상이 유지될 때
- 그 대상이 active이고 destroy 상태가 아닐 때

### `OnMouseHoverExit()`

- 이전 프레임 hover 대상이 있었고
- 이번 프레임 hover 대상이 달라졌을 때
- 이전 대상이 아직 active이고 destroy 상태가 아닐 때

예외:

- hover 대상이 destroy pending이면 `HoverExit` 없이 조용히 hover 상태만 제거될 수 있습니다.
- 에디터에서 ImGui가 마우스를 잡고 있으면 hover를 끊고 editor UI 입력을 우선합니다.

## 7. Component 구현 시 override 지점

사용자 component는 `On...()`를 직접 override하는 것이 아니라 아래 protected virtual을 override하면 됩니다.

- `CollisionEnter(Collider2D* other)`
- `CollisionStay(Collider2D* other)`
- `CollisionExit(Collider2D* other)`
- `TriggerEnter(Collider2D* other)`
- `TriggerStay(Collider2D* other)`
- `TriggerExit(Collider2D* other)`
- `LBtnDown()`
- `LBtnUp()`
- `RBtnDown()`
- `RBtnUp()`
- `MouseHoverEnter()`
- `MouseHoverStay()`
- `MouseHoverExit()`
- `Enable()`
- `Disable()`

예:

```cpp
class MyComponent : public Component
{
protected:
    void Enable() override;
    void Disable() override;
    void CollisionEnter(Collider2D* other) override;
    void MouseHoverEnter() override;
};
```

## 8. 요약

- `OnEnable()`은 생성 이벤트가 아니라 `SetActive(false -> true)` 전환 이벤트입니다.
- `OnDisable()`은 `SetActive(true -> false)` 전환 이벤트입니다.
- active 상태로 처음 생성된 오브젝트는 `OnEnable()`을 자동으로 받지 않습니다.
- collision / trigger `Enter`, `Stay`는 inactive 또는 destroy pending 대상에 전달하지 않습니다.
- `OnCollisionExit(nullptr)`, `OnTriggerExit(nullptr)`는 삭제/강제 종료 정리용 정상 케이스입니다.
- hover와 mouse 입력은 현재 raycast / hover target 기준으로 전달됩니다.
