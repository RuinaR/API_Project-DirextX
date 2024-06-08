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
	float m_updateTimer;
	vector<IDirect3DTexture9*>::iterator m_curItr;

public:
	AnimationRender(Animation anim);

	void Initialize() override;
	void Release() override;
	void ChangeAnim(Animation anim);
	void Start() override;
	void Update() override;
	void SetPlay(bool play);
	void SetOneTime(bool b);
	bool IsFinishAnim(); //oneTime�ɼ��� �������� �� ���������� �ִϸ��̼��� ����Ǿ����� true ��ȯ
	const Animation& GetCurrentAnim();
};

