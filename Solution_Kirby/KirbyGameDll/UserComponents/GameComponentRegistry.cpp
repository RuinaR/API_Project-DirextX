#include "pch.h"
#include "GameComponentRegistry.h"
#include "ComponentFactory.h"
#include "UserComponentRegistration.h"

void RegisterGameComponents(ComponentFactory& factory)
{
	// 사용자 컴포넌트를 등록하는 곳이다.
	// - 새 컴포넌트는 KirbyGameDll/UserComponents/Scripts 아래에 만든다.
	// - 만든 뒤 여기서 ComponentFactory에 등록하면 에디터와 게임에서 쓸 수 있다.
	// - GetSerializableType() 값은 여기서 등록한 type 이름과 같아야 한다.
	// - DrawInspector()는 인스펙터 UI를, Serialize()/Deserialize()는 저장할 값만 다룬다.
	// - 새 C++ 컴포넌트를 추가했다면 DLL을 다시 빌드하고 에디터도 다시 실행해야 한다.
	// - 에디터 실행 중에 C++ 컴포넌트를 바로 바꿔 끼우는 기능은 아직 없다.
	// - 게임 전용 컴포넌트 코드는 EngineFrameworkDll보다 KirbyGameDll에 두는 편이 좋다.
	// 사용 예:
	// REGISTER_GAME_COMPONENT(factory, MyComponent, "My Component", "Game");
}
