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
// - 컴포넌트에 static constexpr const char* kComponentType 를 둔다.
// - GetSerializableType()은 같은 값을 반환한다.
// - displayName/category만 바꿔서 아래 매크로로 등록한다.
// - 새 C++ 컴포넌트 추가 후에는 DLL 재빌드 + 에디터 재실행이 필요하다.
// - 에디터 실행 중 C++ 컴포넌트 hot reload는 지원하지 않는다.
#define REGISTER_GAME_COMPONENT(factory, ComponentType, displayName, category) \
	RegisterUserComponent<ComponentType>((factory), ComponentType::kComponentType, (displayName), (category), true)

#define REGISTER_GAME_COMPONENT_EX(factory, ComponentType, displayName, category, canAddFromEditor) \
	RegisterUserComponent<ComponentType>((factory), ComponentType::kComponentType, (displayName), (category), (canAddFromEditor))
