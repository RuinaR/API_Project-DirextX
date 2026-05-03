#pragma once

#include <string>
#include <type_traits>

#include "Component.h"
#include "ComponentFactory.h"

template <typename TComponent>
bool RegisterUserComponent(
	ComponentFactory& factory,
	const std::string& typeName,
	const std::string& displayName,
	const std::string& category = "Game",
	bool canAddFromEditor = true)
{
	static_assert(std::is_base_of<Component, TComponent>::value, "TComponent must derive from Component.");
	static_assert(std::is_default_constructible<TComponent>::value, "TComponent must be default constructible for helper registration.");

	return factory.Register(
		typeName,
		displayName,
		category,
		canAddFromEditor,
		[](const std::string&) -> Component*
		{
			return new TComponent();
		});
}

// 권장 규칙:
// - 실제 사용자 컴포넌트 구현 파일은 KirbyGameDll/UserComponents/Scripts 아래에 둔다.
// - 컴포넌트 안에는 static constexpr const char* kComponentType 를 둔다.
// - GetSerializableType()는 같은 값을 돌려준다.
// - displayName/category만 바꾸고 싶으면 아래 매크로로 등록하면 된다.
// - 새 C++ 컴포넌트를 추가했다면 DLL을 다시 빌드하고 에디터도 다시 실행해야 한다.
// - 에디터 실행 중 C++ 컴포넌트를 바로 다시 읽는 기능은 아직 없다.
#define REGISTER_GAME_COMPONENT(factory, ComponentType, displayName, category) \
	RegisterUserComponent<ComponentType>((factory), ComponentType::kComponentType, (displayName), (category), true)

#define REGISTER_GAME_COMPONENT_EX(factory, ComponentType, displayName, category, canAddFromEditor) \
	RegisterUserComponent<ComponentType>((factory), ComponentType::kComponentType, (displayName), (category), (canAddFromEditor))
