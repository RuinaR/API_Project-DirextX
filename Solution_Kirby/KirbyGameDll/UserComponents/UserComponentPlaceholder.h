#pragma once

// 사용자 컴포넌트 작업 위치 안내:
// - 새 컴포넌트 클래스는 KirbyGameDll/UserComponents/Scripts 아래에 만든다.
// - ComponentFactory 등록은 RegisterGameComponents()에서 한다.
// - 등록 코드는 UserComponentRegistration.h helper나 macro를 쓰는 편이 편하다.
// - 작업 순서:
//   1) Component를 상속한다.
//   2) static constexpr const char* kComponentType 를 만든다.
//   3) GetSerializableType()가 kComponentType을 돌려주게 한다.
//   4) DrawInspector()에 인스펙터 UI를 구현한다.
//   5) Serialize()/Deserialize()에는 저장할 값만 넣는다.
//   6) UserComponents/GameComponentRegistry.cpp에서 REGISTER_GAME_COMPONENT(...)로 등록한다.
// - 새 C++ 컴포넌트를 반영하려면 DLL을 다시 빌드하고 에디터도 다시 실행해야 한다.
// - 에디터 실행 중 C++ 컴포넌트를 바로 다시 읽는 기능은 아직 없다.
// - EngineFrameworkDll에는 게임 전용 컴포넌트 코드를 넣지 않는다.
