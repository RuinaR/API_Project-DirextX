#pragma once
#include "BitmapRender.h"
#include "Component.h"
#include "structs.h"

class AnimationRender : public Component
{
private:
	Animation m_anim;
	BitmapRender* m_bitren;
	bool m_isPlay;
	bool m_isOneTime = false;
	float m_updateTimer;
	vector<HBITMAP>::iterator m_curItr;

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
};

