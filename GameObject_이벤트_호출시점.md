# GameObject 이벤트 호출 시점

이 문서는 현재 코드 기준으로 `GameObject`와 `Component` 이벤트가 언제 호출되는지 쉽게 정리한 문서입니다.

이벤트는 먼저 `GameObject`가 받고,
그 안에 붙어 있는 `Component`들에게 순서대로 전달됩니다.

## 1. active 관련 이벤트

### `OnEnable()`

호출 시점:

- `SetActive(true)`가 호출될 때
- 이전 상태가 `false`, 새 상태가 `true`일 때만 호출

주의:

- 오브젝트가 처음 만들어졌다고 자동으로 `OnEnable()`이 들어오는 구조는 아닙니다.
- 즉, "생성 알림"이라기보다 "비활성 -> 활성 전환 알림"에 가깝습니다.

### `OnDisable()`

호출 시점:

- `SetActive(false)`가 호출될 때
- 이전 상태가 `true`, 새 상태가 `false`일 때만 호출

## 2. Collision 이벤트

Collision 이벤트는 둘 다 일반 collider일 때 사용합니다.

### `OnCollisionEnter(Collider2D* other)`

- 충돌이 처음 시작될 때

### `OnCollisionStay(Collider2D* other)`

- 충돌 상태가 계속 유지되는 동안

### `OnCollisionExit(Collider2D* other)`

- 충돌이 끝날 때

주의:

- 삭제 예약이나 강제 종료 상황에서는 `other`가 `nullptr`일 수 있습니다.
- 그래서 `OnCollisionExit(nullptr)`도 정상 케이스로 봐야 합니다.

## 3. Trigger 이벤트

둘 중 하나라도 trigger면 trigger 이벤트를 사용합니다.

### `OnTriggerEnter(Collider2D* other)`

- trigger 접촉이 처음 시작될 때

### `OnTriggerStay(Collider2D* other)`

- trigger 접촉 상태가 계속 유지되는 동안

### `OnTriggerExit(Collider2D* other)`

- trigger 접촉이 끝날 때

주의:

- 여기서도 삭제 예약 상황이면 `other == nullptr`가 될 수 있습니다.

## 4. 마우스 버튼 이벤트

현재 hover 중인 오브젝트를 기준으로 들어갑니다.

### `OnLBtnDown()`

- 마우스 왼쪽 버튼 누를 때

### `OnLBtnUp()`

- 마우스 왼쪽 버튼 뗄 때

### `OnRBtnDown()`

- 마우스 오른쪽 버튼 누를 때

### `OnRBtnUp()`

- 마우스 오른쪽 버튼 뗄 때

## 5. Hover 이벤트

### `OnMouseHoverEnter()`

- 이번 프레임에 새로 hover 대상이 되었을 때

### `OnMouseHoverStay()`

- hover 상태가 계속 유지될 때

### `OnMouseHoverExit()`

- 이전 프레임까지 hover 대상이었는데 이번 프레임에는 아니게 되었을 때

주의:

- editor UI가 마우스를 잡고 있는 동안에는 runtime hover 처리가 막힐 수 있습니다.
- destroy 중인 오브젝트는 hover 대상에서 빠질 수 있습니다.

## 6. 이벤트 전달 시 공통 주의점

- 이벤트는 component 목록 snapshot 기준으로 전달됩니다.
- 그래서 이벤트 도중 component 삭제가 일어나도 바로 순회가 깨지지 않게 처리하고 있습니다.
- `DestroyObject()`는 즉시 delete가 아니라 예약 삭제 방식입니다.

## 7. 사용자 컴포넌트에서 override할 때

직접 `On...()` 함수를 건드리는 게 아니라,
아래 protected virtual 함수를 override하는 방식입니다.

예:

```cpp
class MyComponent : public Component
{
protected:
    void CollisionEnter(Collider2D* other) override;
    void TriggerExit(Collider2D* other) override;
    void MouseHoverEnter() override;
    void Enable() override;
    void Disable() override;
};
```

## 8. 한 줄 정리

이벤트는 대부분
"GameObject가 먼저 받고 -> 붙어 있는 Component에 나눠 주는 구조"이며,
삭제 예약이나 inactive 상태를 고려해서 조금 보수적으로 전달되도록 정리되어 있습니다.