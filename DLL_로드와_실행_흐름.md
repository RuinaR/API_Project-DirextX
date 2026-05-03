# 사용자 컴포넌트 / 액션 가이드

이 문서는 `KirbyGameDll`에 사용자 컴포넌트와 사용자 액션을 추가할 때
기본적으로 어떤 흐름으로 작업하면 되는지 정리한 문서입니다.

어렵게 생각하기보다,
`게임 코드만 따로 넣는 자리`라고 생각하면 이해하기 쉽습니다.

## 1. 어디에 추가하는가

사용자 컴포넌트는 보통 아래 폴더에 만듭니다.

```text
KirbyGameDll/UserComponents/Scripts
```

사용자 액션은 보통 아래 폴더에 만듭니다.

```text
KirbyGameDll/UserActions/Scripts
```

## 2. 사용자 컴포넌트 기본 흐름

새 컴포넌트를 만들 때는 보통 아래 순서로 진행합니다.

1. `MyComponent.h`, `MyComponent.cpp` 파일을 만든다.
2. `Component`를 상속한다.
3. `kComponentType`를 정한다.
4. `GameComponentRegistry.cpp`에 등록한다.
5. 필요하면 Inspector, 저장/로드, 참조 필드를 구현한다.

기본 형태 예시:

```cpp
class MyComponent : public Component
{
public:
    static constexpr const char* kComponentType = "MyComponent";

    void Initialize() override;
    void Release() override;
    void Start() override;
    void Update() override;

    const char* GetInspectorName() const override;
    const char* GetSerializableType() const override;
};
```

## 3. 꼭 맞춰야 하는 이름

아래 값은 서로 맞춰 두는 편이 좋습니다.

- `kComponentType`
- `GetSerializableType()` 반환값
- `GameComponentRegistry.cpp`에 등록하는 type 이름

이 값이 다르면 저장/로드할 때 헷갈릴 수 있습니다.

## 4. 컴포넌트 등록

새 컴포넌트는 아래 파일에서 등록합니다.

```text
KirbyGameDll/UserComponents/GameComponentRegistry.cpp
```

예시:

```cpp
REGISTER_GAME_COMPONENT(factory, MyComponent, "My Component", "User");
```

의미는 보통 이 정도로 보면 됩니다.

- 실제 C++ 타입
- 에디터에 보일 이름
- 카테고리 이름

## 5. Inspector 만들기

에디터에서 값을 조절하고 싶으면 `DrawInspector()`를 구현합니다.

예시:

```cpp
void MyComponent::DrawInspector()
{
    SyncImGuiContextForCurrentModule();
    ImGui::Checkbox("Enabled", &m_enabled);
    ImGui::DragFloat("Speed", &m_speed, 0.1f);
}
```

주의:

- `KirbyGameDll` 쪽 Inspector에서는 보통
  `SyncImGuiContextForCurrentModule();`
  를 먼저 호출하는 편이 안전합니다.

## 6. 저장/로드가 필요하면

씬 저장이 필요하면 아래 함수를 같이 구현합니다.

- `Serialize()`
- `Deserialize()`

모든 값을 다 저장할 필요는 없습니다.
다시 계산 가능한 값은 빼고,
다음 실행 때 다시 필요한 값만 저장하는 편이 좋습니다.

## 7. 참조 필드가 필요하면

`GameObject*`, `Component*` 같은 참조를 Inspector에 보이고 싶으면
`RegisterReferenceFields(...)`를 구현합니다.

예시:

```cpp
void MyComponent::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
    registry.RegisterGameObjectRef("Target Object", &m_targetObject);
    registry.RegisterComponentRef("Target Rigidbody", &m_targetComponent, "Rigidbody2D");
}
```

이렇게 하면 Inspector에서 공통 방식으로 필드가 그려집니다.

현재 지원되는 쪽은 보통 아래와 같습니다.

- `GameObject` 참조 필드
- `Component` 참조 필드
- hierarchy에서 `GameObject` drag & drop
- 타입이 맞는 component 자동 연결
- SceneData 저장/로드

## 8. 사용자 액션 기본 흐름

사용자 액션은 아래 파일에서 등록합니다.

```text
KirbyGameDll/UserActions/GameActionRegistry.cpp
```

현재 버튼 액션은 보통
SceneData에 함수 본체를 저장하는 방식이 아니라,
`actionKey`만 저장하고 시작할 때 registry에서 다시 연결하는 방식입니다.

예시:

```cpp
UIActionRegistry::RegisterAction(
    "SampleButtonAction",
    []() { ExecuteSampleButtonAction(); });
```

즉:

- SceneData에는 문자열 key 저장
- 실제 동작은 시작할 때 다시 등록

구조로 이해하면 됩니다.

## 9. TimeScale과 DeltaTime 사용 기준

사용자 컴포넌트에서 시간을 쓸 때는 아래 기준으로 보면 됩니다.

- 기본값은 `MainFrame::GetInstance()->DeltaTime()` 입니다.
- 이 값은 `TimeScale` 영향을 받습니다.
- 그래서 일반 게임 플레이 로직은 이 값을 쓰는 쪽이 의도에 맞습니다.

예:

- 캐릭터 이동
- 점프 보정
- 일반 타이머
- 적 패턴 진행
- 스크롤 속도 반영

반대로 `MainFrame::GetInstance()->UnscaledDeltaTime()`은
`TimeScale` 영향을 받지 않습니다.

이 값은 아래처럼 특별한 연출에만 쓰는 편이 좋습니다.

- 게임이 잠깐 멈춘 것처럼 보여도 계속 돌아야 하는 연출
- 슬로우/정지 연출과 무관하게 진행해야 하는 UI 효과
- 짧은 컷신 느낌의 보정

정리하면:

- 기본 게임 로직: `DeltaTime()`
- 타임스케일 무시 연출: `UnscaledDeltaTime()`

최근 샘플 게임의 아이템 획득 연출도 이 기준을 따릅니다.

## 10. 빌드할 때 주의할 점

현재 사용자 C++ 컴포넌트 / 액션은 hot reload를 지원하지 않습니다.

즉 보통은:

1. 코드 수정
2. DLL 다시 빌드
3. 에디터 다시 실행

순서로 보는 쪽이 안전합니다.

## 11. 시간값을 쓸 때 체크할 것

사용자 컴포넌트에서 시간 관련 로직을 넣을 때는 먼저 아래를 정합니다.

- 이 로직이 `TimeScale` 영향을 받아야 하는지
- 아니면 `TimeScale`과 무관하게 계속 돌아야 하는지

기본 기준은 아래처럼 보면 됩니다.

- 일반 로직: `DeltaTime()`
- 예외 연출: `UnscaledDeltaTime()`

예를 들어:

- 이동, 점프, 스크롤, 일반 쿨타임은 `DeltaTime()`
- 잠깐 멈춤 연출 중에도 계속 돌아야 하는 회전/이펙트는 `UnscaledDeltaTime()`

헷갈리면 먼저 `DeltaTime()`으로 만들고,
정말 타임스케일을 무시해야 할 때만 `UnscaledDeltaTime()`으로 바꾸는 편이 안전합니다.
