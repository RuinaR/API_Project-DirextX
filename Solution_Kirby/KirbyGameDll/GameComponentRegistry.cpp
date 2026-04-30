#include "pch.h"
#include "GameComponentRegistry.h"
#include "ComponentFactory.h"
#include "Door.h"
#include "FBXRotateObj.h"

void RegisterGameComponents(ComponentFactory& factory)
{
	// 사용자 컴포넌트 에디터 노출 최소 규칙:
	// 1) KirbyGameDll에 컴포넌트를 작성한다.
	// 2) 여기서 ComponentFactory에 typeName으로 등록한다.
	// 3) GetSerializableType()은 등록한 typeName과 같은 값을 반환한다.
	// 4) DrawInspector()에서 에디터 편집 항목을 그린다.
	// 5) Serialize()/Deserialize()에는 런타임 포인터 없이 저장할 값만 넣는다.
	factory.Register("Door", "Door", "Gameplay", true, [](const std::string&) -> Component*
		{
			return new Door();
		});

	factory.Register(FBXRotateObj::kComponentType, "FBX Rotate Object", "Gameplay", true, [](const std::string&) -> Component*
		{
			return new FBXRotateObj();
		});

	// 런타임 문맥, 생성자 인자, 싱글톤 소유권이 필요한 컴포넌트는
	// 전용 생성 함수와 인스펙터/직렬화 지원을 갖춘 뒤 나중에 등록한다.
}
