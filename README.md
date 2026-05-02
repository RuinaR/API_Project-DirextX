# KirbyEngine

C++17, Win32API, DirectX9, ImGui, Box2D, FBX SDK 기반으로 만드는 미니 게임엔진/에디터 프로젝트입니다.

## 현재 프로젝트 상태

- `EngineFramework` 안에 런타임과 에디터 코드가 함께 들어 있습니다.
- `KirbyGameDll`은 게임별 `Component`, `Action`, bootstrap 코드를 두는 확장 계층입니다.
- 실행 경로는 크게 둘입니다.
  - `BuildEdit`: 에디터 실행기
  - `BuildResultGame`: 게임 실행기
- 둘 다 `KirbyGameDll.dll`을 로드해 공통 게임 초기화를 사용합니다.

## 장기 구조 목표

- `EngineRuntime`
  - 게임 실행에 필요한 기능만 포함
- `EngineEditor`
  - ImGui, Hierarchy, Inspector, BuildSettings, Scene 편집 기능 포함
- `KirbyGameDll`
  - 게임별 `Component`, `Action`만 포함
  - 장기적으로 `EngineRuntime`만 의존
- `EditorApp`
  - `EngineRuntime + EngineEditor + KirbyGameDll`
- `GameApp`
  - `EngineRuntime + KirbyGameDll`

현재는 아직 완전히 분리되지 않았고, `MainFrame`, `WindowFrame`, `RenderManager`, `ObjectManager`, `SceneDataManager`에 Runtime/Editor 책임이 함께 섞여 있습니다.

## 현재 핵심 구조

### 런타임

- `GameObject + Component` 구조
- local/world transform hierarchy 지원
- parent-child 관계 유지 시 world transform 보존
- `ObjectManager`가 object lifecycle, pending add/remove, mouse raycast hover, SceneData 복원을 담당
- `RenderManager`가 월드 렌더, UI 렌더, debug render를 담당
- `MainFrame`이 DX9 device, frame loop, physics step, collision dispatch, resize/reset을 담당

### 에디터

- ImGui 기반 편집 UI 사용
- 현재 `Hierarchy` 창 이름은 `Editor`로 변경됨
- `Editor` 창 내부의 하이어라키 구역은 접기/펼치기 가능
- `Hierarchy / Inspector / ResourceBrowser / Scene 저장/열기 / BuildSettings / Camera 편집` 기능이 이미 들어가 있음

### 물리

- 2D 전용 `Physics2D` 구조 사용
- 주요 타입:
  - `Rigidbody2D`
  - `Collider2D`
  - `BoxCollider2D`
  - `CircleCollider2D`
- 실제 충돌 이벤트는 Box2D `ContactListener` 기반
- `fixture userData -> Collider2D*` 경로로 `GameObject / Component` 이벤트를 전달

## SceneData 상태

### 현재 저장 버전

- 최신 저장 버전: `6`

### 현재 저장 구조

- 최상위:
  - `version`
  - `sceneName`
  - `timeScale`
  - `camera`
  - `objects`
- object:
  - `id`
  - `parentId`
  - `name`
  - `tag`
  - `active`
  - `transform`
  - `components`
- component:
  - `{ "type": "...", "data": { ... } }`

### 저장하는 것

- scene 이름
- timeScale
- camera position / rotation / projection 설정
- object id / parentId / name / tag / active / transform
- component type 문자열
- component별 직렬화 데이터
- asset key / path 문자열
- user action의 `actionKey`
- object reference id

### 저장하지 않는 것

- raw pointer
- `IDirect3DTexture9*`
- `b2Body*`, fixture pointer
- callback / lambda 본체
- DirectX runtime object
- Box2D runtime object
- editor UI 상태
- raycast debug 상태

### 호환 정책

- v3 SceneData도 계속 로드 가능
- `BoxCollider2D`가 canonical type
- legacy `"BoxCollider"` alias는 구버전 SceneData 호환용으로 계속 로드 가능
- 현재 원본 `SceneData/`에는 legacy `"BoxCollider"` 문자열이 남아 있지 않음

## Physics2D 상태 요약

- `Rigidbody2D`가 있으면 collider는 그 body에 fixture로 붙습니다.
- `Rigidbody2D`가 없으면 collider가 static fallback body를 직접 소유합니다.
- collider만 삭제하면 fixture만 제거되고 body는 남을 수 있습니다.
- `Rigidbody2D`만 삭제하면 collider는 fallback body로 복귀합니다.
- `Physics2D::Raycast`는 `BoxCollider2D`, `CircleCollider2D`를 지원합니다.
- `CollisionEnter/Stay/Exit`는 둘 다 non-trigger일 때만 호출됩니다.
- 둘 중 하나라도 trigger면 `TriggerEnter/Stay/Exit`가 호출됩니다.
- 삭제 예약 또는 강제 종료 상황에서는 `OnCollisionExit(nullptr)`, `OnTriggerExit(nullptr)`를 허용합니다.

## 현재 샘플 / 테스트 씬 분류

### 예시 씬

- `SceneData/Sample.json`
  - Sprite / Animation / UI / FBX / collision visual 예시

### 검증 씬

- `SceneData/Physics2DTestScene.json`
  - 기본 Physics2D 검증
- `SceneData/Physics2DInteractionTestScene.json`
  - Dynamic / Kinematic / Trigger interaction 검증
- `SceneData/ActivePhysicsToggleTestScene.json`
  - Active / Trigger / UI action 연동 검증

## 현재 테스트 / 샘플 컴포넌트 정책

### 샘플 유지

- `SampleSpinComponent`
- `Physics2DKeyboardTestComponent`
- `CollisionVisualTestComponent`

### 임시 테스트 유지

- `ActivePhysicsTestStatusComponent`
- `ActivePhysicsTestInfoComponent`

자세한 정책은 `UserExtensionGuide.md`를 참고하면 됩니다.

## 최근 구조 정리 반영 사항

- `CollisionManager.h/.cpp`는 더 이상 활성 충돌 시스템이 아님
- `EngineFramework.vcxproj`, `EngineFramework.vcxproj.filters`에서는 `CollisionManager` 항목 제거 완료
- 디스크에는 deprecated 후보 파일로만 남아 있음
- 실제 충돌 흐름은 `MainFrame::CollisionListener` 기반

## 현재 남아 있는 구조적 혼합 지점

### Runtime 쪽에 남아 있는 Editor 책임 흔적

- `MainFrame`
  - ImGui 초기화
  - editor playback (`Play / Pause / Step`)
- `RenderManager`
  - `EditUpdate()`
  - selected object marker
  - editor frame window
- `ObjectManager`
  - `ImguiUpdate()`
  - editor windows 직접 호출
- `SceneDataManager`
  - dirty / snapshot
  - open / new / save-as 같은 editor workflow
- `WindowFrame`
  - scene open 정책과 SceneData load failure UI 처리

## 문서 안내

- 현재 구조/정책 요약:
  - `README.md`
  - `프로젝트_정책_및_가이드.md`
- 사용자 확장과 테스트/샘플 정책:
  - `UserExtensionGuide.md`
- Physics2D 상태:
  - `Physics2D_Status.md`
- 이벤트 호출 시점:
  - `GameObject_이벤트_호출시점.md`
- 정책을 택한 이유와 문제 해결 경험:
  - `설계_배경_및_문제해결_기록.md`
