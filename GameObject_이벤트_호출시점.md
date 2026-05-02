# GameObject 이벤트 호출 시점 정리

이 문서는 현재 `KirbyEngine/Solution_Kirby` 코드 기준으로 `GameObject`와 각 `Component`에 전달되는 주요 이벤트의 **정확한 호출 시점**을 정리한 문서입니다.

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

---

## 1. 공통 전달 방식

위 이벤트들은 모두 `GameObject`가 먼저 받고, 그 안에서 현재 붙어 있는 모든 `Component`에게 순서대로 전달됩니다.

전달 방식 특징:

- `GameObject`는 component 목록의 snapshot을 만든 뒤 순회합니다.
- 이벤트 도중 어떤 component가 다른 component를 제거해도 순회가 최대한 안전하게 유지되도록 구성되어 있습니다.
- 일부 이벤트는 처리 중 `DestroyObject()`가 예약되면 이후 component 전달을 중단합니다.
- `OnCollisionExit`, `OnTriggerExit`는 종료 성격 이벤트라서 destroy 예약 중에도 남은 component에 계속 전달될 수 있습니다.

---

## 2. Active 전환 이벤트

### `OnEnable()`

호출 시점:

- `GameObject::SetActive(true)` 호출 시
- 이전 상태가 `false`였고 새 상태가 `true`일 때만 호출

중요:

- 현재 엔진의 `OnEnable()`은 **유니티처럼 "객체가 처음 생성될 때 자동 호출되는 이벤트"가 아니다.**
- 즉, `new GameObject()` 직후에도 자동 호출되지 않는다.
- `InitializeSet()`, `Start()`, `AddComponent()` 시점에도 자동 호출되지 않는다.
- SceneData 로드 중 active가 `true`인 오브젝트가 생성되어도 자동 호출되지 않는다.
- **오직 `SetActive(false)` 상태였다가 `SetActive(true)`로 다시 바뀌는 순간에만 호출된다.**

현재 동작 순서:

1. 기존 active 상태를 기억
2. `m_setActive = true` 반영
3. `false -> true` 전환이면 `OnEnable()` 호출
4. `GameObject`가 붙어 있는 모든 component에 `component->OnEnable()` 전달

호출되지 않는 경우:

- 이미 active인 상태에서 다시 `SetActive(true)`를 호출한 경우
- 처음 생성된 직후
- 씬 로드로 active 상태 오브젝트가 처음 만들어진 경우
- 오브젝트가 destroy 상태인 경우
- component 목록이 없는 경우

주의:

- `OnEnable()`이 호출될 때는 `GetActive()`가 이미 `true`를 반환합니다.
- 현재 의미는 "생성 시 초기화"가 아니라 **"비활성 -> 활성 전환 알림"**에 가깝습니다.
- 활성화 시 physics body와 fixture도 다시 활성화되며, 이미 다른 collider와 겹친 상태라면 다음 physics step에서 정상적으로 `CollisionEnter` 또는 `TriggerEnter`가 다시 들어올 수 있습니다.

### `OnDisable()`

호출 시점:

- `GameObject::SetActive(false)` 호출 시
- 이전 상태가 `true`였고 새 상태가 `false`일 때만 호출

현재 동작 순서:

1. 기존 active 상태를 기억
2. `m_setActive = false` 반영
3. 이 시점부터 collision/trigger의 새 `Enter`/`Stay` 전달 조건에서 제외
4. physics 정리 과정에서 기존 contact pair가 정리되며 필요한 `CollisionExit` / `TriggerExit`가 전달될 수 있음
5. `true -> false` 전환이면 `OnDisable()` 호출
6. `GameObject`가 붙어 있는 모든 component에 `component->OnDisable()` 전달

호출되지 않는 경우:

- 이미 inactive인 상태에서 다시 `SetActive(false)`를 호출한 경우
- 오브젝트가 destroy 상태인 경우
- component 목록이 없는 경우

주의:

- `OnDisable()`이 호출될 때는 `GetActive()`가 이미 `false`를 반환합니다.
- 현재 구조에서는 `SetActive(false)` 시 collider/body도 함께 비활성화됩니다.
- 이 과정에서 기존 contact pair는 정리되며 필요한 `CollisionExit` / `TriggerExit`가 전달됩니다.
- 구현상 `OnDisable()`보다 `CollisionExit` / `TriggerExit`가 먼저 갈 수 있습니다.
- 현재 의미는 **"활성 -> 비활성 전환 알림"**입니다.

---

## 3. Collision 이벤트

Collision 이벤트는 Box2D contact callback 기준으로 들어옵니다.

정책:

- 두 fixture가 모두 non-trigger일 때만 collision 계열 이벤트가 호출됩니다.
- 둘 중 하나라도 trigger(sensor)면 collision이 아니라 trigger 계열 이벤트로 처리됩니다.

### `OnCollisionEnter(Collider2D* col)`

호출 시점:

- Box2D `BeginContact`가 발생했을 때
- 접촉한 두 collider가 모두 non-trigger일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 delete-pending이면 전달하지 않음
- receiver 또는 other 쪽 `GameObject`가 inactive면 전달하지 않음

주의:

- 현재 구조에서는 `SetActive(false)` 시 physics body 자체가 비활성화되므로 inactive 상태에서는 새 collision 이벤트가 들어오지 않습니다.

### `OnCollisionStay(Collider2D* col)`

호출 시점:

- Box2D `PreSolve` 단계에서
- 접촉이 계속 유지 중이고 non-trigger contact일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 delete-pending이면 전달하지 않음
- receiver 또는 other 쪽 `GameObject`가 inactive면 전달하지 않음

주의:

- inactive 상태에서는 physics body가 비활성화되어 `CollisionStay`도 계속되지 않습니다.

### `OnCollisionExit(Collider2D* col)`

호출 시점:

- Box2D `EndContact`가 발생했을 때
- 또는 active collision pair 정리 과정에서 강제 종료가 감지됐을 때

전달 조건:

- receiver 쪽 `GameObject`가 이미 destroy 상태면 전달하지 않음

`col` 인자 정책:

- 상대 오브젝트가 정상 상태면 상대 `Collider2D*`
- 상대 오브젝트가 이미 delete-pending 또는 destroy 상태면 `nullptr`

즉:

- `OnCollisionExit(nullptr)`는 삭제 예약/강제 종료 상황을 의미할 수 있습니다.

---

## 4. Trigger 이벤트

Trigger 이벤트는 Box2D sensor contact 기준으로 들어옵니다.

정책:

- 둘 중 하나라도 trigger(sensor)면 양쪽 모두 trigger 이벤트를 받습니다.

### `OnTriggerEnter(Collider2D* col)`

호출 시점:

- Box2D `BeginContact`가 발생했을 때
- 두 fixture 중 하나 이상이 trigger일 때

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 delete-pending이면 전달하지 않음
- inactive 상태에서는 physics body가 비활성화되어 새 `TriggerEnter`는 들어오지 않습니다.
- active가 false로 바뀐 직후에는 body 비활성화가 지연 적용 중이어도 `TriggerEnter` / `TriggerStay` 전달 조건에서 제외됩니다.

### `OnTriggerStay(Collider2D* col)`

호출 시점:

- 매 프레임 `DispatchActiveTriggerStayEvents()`에서
- active trigger pair로 추적 중인 접촉에 대해 호출

전달 조건:

- receiver 또는 other 쪽 `GameObject`가 delete-pending이면 stay 대신 exit 정리로 넘어감
- inactive 상태에서는 physics body가 비활성화되어 `TriggerStay`도 계속되지 않습니다.

주의:

- TriggerStay는 Box2D `PreSolve`가 아니라 엔진 쪽 active pair 추적 루프에서 호출됩니다.

### `OnTriggerExit(Collider2D* col)`

호출 시점:

- Box2D `EndContact`가 발생했을 때
- 또는 tracked trigger pair 정리 중 삭제/강제 종료가 감지됐을 때

전달 조건:

- receiver 쪽 `GameObject`가 이미 destroy 상태면 전달하지 않음

`col` 인자 정책:

- 상대 오브젝트가 정상 상태면 상대 `Collider2D*`
- 상대 오브젝트가 이미 delete-pending 또는 destroy 상태면 `nullptr`

즉:

- `OnTriggerExit(nullptr)`는 삭제 예약/강제 종료 상황을 의미할 수 있습니다.

---

## 5. 마우스 버튼 이벤트

마우스 버튼 이벤트는 Win32 메시지에서 시작됩니다.

흐름:

1. `WindowFrame::WndProc`
2. `ObjectManager::OnLBtnDown / OnLBtnUp / OnRBtnDown / OnRBtnUp`
3. 현재 hover 중인 오브젝트를 대상으로 전달
4. `GameObject`가 각 component에 전달

공통 조건:

- 현재 마우스가 가리키는 오브젝트여야 함
- 오브젝트가 active여야 함
- 오브젝트가 destroy 상태가 아니어야 함

### `OnLBtnDown()`

호출 시점:

- `WM_LBUTTONDOWN` 메시지를 받았을 때
- 현재 hover 타겟이 유효할 때

### `OnLBtnUp()`

호출 시점:

- `WM_LBUTTONUP` 메시지를 받았을 때
- 현재 hover 타겟이 유효할 때

### `OnRBtnDown()`

호출 시점:

- `WM_RBUTTONDOWN` 메시지를 받았을 때
- 현재 hover 타겟이 유효할 때

### `OnRBtnUp()`

호출 시점:

- `WM_RBUTTONUP` 메시지를 받았을 때
- 현재 hover 타겟이 유효할 때

주의:

- 클릭 처리 중 오브젝트가 destroy 예약되면 이후 hover 타겟을 비워서 뒤이은 입력/hover stay가 가지 않도록 정리합니다.

---

## 6. 마우스 Hover 이벤트

Hover 이벤트는 `ObjectManager::UpdateMouseInteraction()`에서 갱신됩니다.

이 함수는:

- 일반 게임/플레이 중에는 `ObjectManager::Update()` 안에서 매 프레임 실행
- 에디터 `Paused` 상태에서도 별도로 계속 실행

즉:

- 플레이가 멈춰 있어도 에디터에서는 hover 이벤트가 계속 갱신될 수 있습니다.

### `OnMouseHoverEnter()`

호출 시점:

- 이번 프레임 raycast 결과의 hover 대상이 이전 프레임 대상과 달라졌고
- 새 대상이 active이며 destroy 상태가 아닐 때

### `OnMouseHoverStay()`

호출 시점:

- 이번 프레임에도 같은 hover 대상이 유지되고
- 그 대상이 active이며 destroy 상태가 아닐 때

### `OnMouseHoverExit()`

호출 시점:

- 이전 프레임 hover 대상이 있었고
- 이번 프레임 hover 대상이 달라졌을 때
- 이전 대상이 아직 active이며 destroy 상태가 아닐 때

중요 예외:

- hover 대상이 destroy 예약되면 `HoverExit` 없이 조용히 hover 상태만 해제됩니다.
- hover 대상이 inactive가 된 경우도 현재 구현상 `HoverExit`가 항상 보장되지는 않습니다.

---

## 7. Component 구현 방법

사용자 component에서는 `On...()`를 직접 override하는 것이 아니라 아래 protected virtual을 override하면 됩니다.

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

---

## 8. 현재 코드 기준 핵심 요약

- `OnEnable()`은 생성 시 이벤트가 아니라 `SetActive(false -> true)` 직후 호출됩니다.
- `OnDisable()`은 `SetActive(true -> false)` 직후 호출됩니다.
- active 상태로 처음 생성된 오브젝트는 `OnEnable()`을 자동으로 받지 않습니다.
- mouse 이벤트는 active 오브젝트에만 들어갑니다.
- hover 이벤트도 active 오브젝트 기준으로만 들어갑니다.
- `SetActive(false)`가 되면 physics body도 비활성화되므로 inactive 상태에서는 새 collision/trigger 이벤트가 들어오지 않습니다.
- `OnCollisionExit(nullptr)`, `OnTriggerExit(nullptr)`는 정상 케이스가 아니라 삭제/강제 종료 정리 케이스일 수 있습니다.
