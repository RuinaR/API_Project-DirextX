#pragma once

// 사용자 컴포넌트 작업 위치:
// - 새 Component 클래스는 KirbyGameDll/UserComponents 아래에 둔다.
// - ComponentFactory 등록은 RegisterGameComponents()에서 한다.
// - 등록 코드는 UserComponentRegistration.h helper/macro 사용을 권장한다.
// - 작성 절차:
//   1) Component를 상속한다.
//   2) static constexpr const char* kComponentType 를 작성한다.
//   3) GetSerializableType()이 kComponentType을 반환하게 한다.
//   4) DrawInspector()에 에디터 편집 항목을 구현한다.
//   5) Serialize()/Deserialize()에 저장 가능한 값만 구현한다.
//   6) GameComponentRegistry.cpp에서 REGISTER_GAME_COMPONENT(...)로 등록한다.
// - Serialize/Deserialize에는 런타임 포인터 대신 저장 가능한 값만 넣는다.
// - 새 C++ 컴포넌트 반영은 DLL 재빌드 + 에디터 재실행 정책으로 간다.
// - 에디터 실행 중 C++ 컴포넌트 hot reload는 지원하지 않는다.
// - EngineFramework에는 게임별 사용자 컴포넌트 코드를 넣지 않는다.
