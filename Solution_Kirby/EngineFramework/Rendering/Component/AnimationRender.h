#pragma once
#include "ImageRender.h"
#include "Component.h"
#include "structs.h"

class AnimationRender : public Component
{
private:
	Animation m_anim;
	ImageRender* m_bitren;
	bool m_isPlay;
	bool m_isOneTime = false;
	std::string m_animationFolderKey;
	AnimationClipData m_clip;
	std::vector<AnimationRuntimeFrame> m_runtimeFrames;
	size_t m_runtimeFrameIndex = 0;
	float m_frameDuration = 0.1f;
	bool m_loop = true;
	bool m_playOnStart = true;
	bool m_useColorKey = true;
	float m_updateTimer;
	vector<IDirect3DTexture9*>::iterator m_curItr;
	bool m_released = false;
	bool m_reloadRequested = false;

	ImageRender* ResolveImageRender();

public:
	AnimationRender(Animation anim);

	void Initialize() override;
	void Release() override;
	void ChangeAnim(Animation anim);
	void Start() override;
	void Update() override;
	void SetPlay(bool play);
	void SetOneTime(bool b);
	bool IsFinishAnim(); //oneTime옵션이 켜져있을 때 마지막까지 애니메이션이 재생되었으면 true 반환
	const Animation& GetCurrentAnim();
	const std::string& GetAnimationFolderKey() const;
	void SetAnimationFolderKey(const std::string& animationFolderKey);
	void ReloadFromFolder();
	void RequestReloadFromFolder();
	void RebuildClipFromFolder();
	void RebuildRuntimeFrames();
	const AnimationClipData& GetClipData() const;
	const std::vector<AnimationRuntimeFrame>& GetRuntimeFrames() const;
	float GetFrameDuration() const;
	void SetFrameDuration(float frameDuration);
	bool GetLoop() const;
	void SetLoop(bool loop);
	bool GetPlayOnStart() const;
	void SetPlayOnStart(bool playOnStart);
	bool GetUseColorKey() const;
	void SetUseColorKey(bool useColorKey);
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
