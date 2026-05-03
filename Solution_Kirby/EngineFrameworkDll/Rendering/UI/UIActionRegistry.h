#pragma once
#include "../../EngineFrameworkAPI.h"

class UIButton;

class ENGINEFRAMEWORK_API UIActionRegistry
{
private:
	static std::map<std::string, std::function<void()>>& GetActions();

public:
	// UI 액션 정책:
	// - 실제 action callback 정의와 등록은 KirbyGameDll 같은 게임 프로젝트에서 한다.
	// - EngineFrameworkDll은 actionKey 기반 registry 연결 방식만 제공한다.
	// - SceneData에는 callback이나 lambda 대신 actionKey 문자열만 저장한다.
	static void RegisterAction(const std::string& actionKey, std::function<void()> action);
	static void UnregisterAction(const std::string& actionKey);
	static void Clear();
	static bool Bind(UIButton* button, const std::string& actionKey);
	static std::vector<std::string> GetRegisteredActionKeys();
};
