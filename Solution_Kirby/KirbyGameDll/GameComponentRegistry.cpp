#include "pch.h"
#include "GameComponentRegistry.h"
#include "ComponentFactory.h"
#include "Door.h"
#include "FBXRotateObj.h"

void RegisterGameComponents(ComponentFactory& factory)
{
	factory.Register("Door", "Door", "Gameplay", true, [](const std::string&) -> Component*
		{
			return new Door();
		});

	factory.Register("FBXRotateObj", "FBX Rotate Object", "Gameplay", true, [](const std::string&) -> Component*
		{
			return new FBXRotateObj();
		});

	// 런타임 문맥, 생성자 인자, 싱글톤 소유권이 필요한 컴포넌트는
	// 전용 생성 함수와 인스펙터/직렬화 지원을 갖춘 뒤 나중에 등록한다.
}
