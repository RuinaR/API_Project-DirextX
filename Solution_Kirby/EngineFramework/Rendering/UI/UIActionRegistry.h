#pragma once

class UIButton;

class UIActionRegistry
{
private:
	static std::map<std::string, std::function<void()>>& GetActions();

public:
	// UI 이벤트 정책:
	// - 실제 action callback 정의/등록은 KirbyGameDll 같은 게임 프로젝트에서 수행한다.
	// - EngineFramework는 actionKey 기반 registry/bind 구조만 제공한다.
	// - SceneData에는 callback/lambda를 저장하지 않고 actionKey 문자열만 저장한다.
	static void RegisterAction(const std::string& actionKey, std::function<void()> action);
	static void UnregisterAction(const std::string& actionKey);
	static void Clear();
	static bool Bind(UIButton* button, const std::string& actionKey);
	static std::vector<std::string> GetRegisteredActionKeys();
};
