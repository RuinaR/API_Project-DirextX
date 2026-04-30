#include "pch.h"
#include "GameComponentRegistry.h"
#include "ComponentFactory.h"
#include "Scripts/SampleSpinComponent.h"
#include "UserComponentRegistration.h"

void RegisterGameComponents(ComponentFactory& factory)
{
	// UserComponents 등록 전용 지점:
	// 새 사용자 컴포넌트는 KirbyGameDll/UserComponents/Scripts 아래에 두고
	// 여기서 ComponentFactory에 등록한 뒤 DLL 재빌드/에디터 재실행으로 반영한다.
	// 사용자 컴포넌트 에디터 노출 최소 규칙:
	// 1) KirbyGameDll/UserComponents/Scripts에 컴포넌트를 작성한다.
	// 2) 여기서 ComponentFactory에 typeName으로 등록한다.
	// 3) GetSerializableType()은 등록한 typeName과 같은 값을 반환한다.
	// 4) DrawInspector()에서 에디터 편집 항목을 그린다.
	// 5) Serialize()/Deserialize()에는 런타임 포인터 없이 저장할 값만 넣는다.
	// 사용 예:
	// REGISTER_GAME_COMPONENT(factory, MyComponent, "My Component", "Game");
	// 정책:
	// - 새 C++ 컴포넌트 추가 후에는 DLL 재빌드 + 에디터 재실행이 필요하다.
	// - 에디터 실행 중 C++ 컴포넌트 hot reload는 지원하지 않는다.
	// - EngineFramework에는 게임별 사용자 컴포넌트 코드를 넣지 않는다.
	REGISTER_GAME_COMPONENT(factory, SampleSpinComponent, "Sample Spin", "User");
	// 런타임 문맥, 생성자 인자, 싱글톤 소유권이 필요한 사용자 컴포넌트는
	// 전용 생성 함수와 인스펙터/직렬화 지원을 갖춘 뒤 여기서 다시 등록한다.
}
