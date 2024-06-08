#include "pch.h"
#include "AnimationRender.h"
#include "ImageRender.h"
#include "GameObject.h"

AnimationRender::AnimationRender(Animation anim)
	:Component(), m_isPlay(true), m_updateTimer(0.0f), m_curItr(), m_bitren(nullptr)
{
	m_anim = anim;
}

void AnimationRender::Initialize()
{
	m_bitren = m_gameObj->GetComponent<ImageRender>();
	if (m_bitren == nullptr)
	{
		m_bitren = new ImageRender(nullptr);
		m_gameObj->AddComponent(m_bitren);
	}
}

void AnimationRender::Release()
{
	m_bitren = nullptr;
}

void AnimationRender::ChangeAnim(Animation anim)
{
	if (anim.textures.size() == 0)
	{
		cout << "animation err " << endl;
		return;
	}
	m_anim = anim;
	m_curItr = m_anim.textures.begin();
	m_bitren->ChangeTexture(*m_curItr);
}

void AnimationRender::Start()
{
	m_curItr = m_anim.textures.begin();
}

void AnimationRender::Update()
{
	if (m_anim.textures.size() == 0)
	{
		cout << "animation err " << endl;
		return;
	}

	if (!m_isPlay)
		return;

	m_updateTimer += MainFrame::GetInstance()->DeltaTime();
	if (m_updateTimer < (float)m_anim.time)
		return;

	if (m_isOneTime && m_curItr == --m_anim.textures.end())
		return;
	m_updateTimer = 0.0f;
	m_curItr++;
	if (m_curItr == m_anim.textures.end())
	{
		m_curItr = m_anim.textures.begin();
	}

	m_bitren->ChangeTexture(*m_curItr);
}

void AnimationRender::SetPlay(bool play)
{
	m_isPlay = play;
}

void AnimationRender::SetOneTime(bool b)
{
	m_isOneTime = b;
}

bool AnimationRender::IsFinishAnim()
{
	if (m_isOneTime && m_curItr == --m_anim.textures.end())
		return true;

	return false;
}

const Animation& AnimationRender::GetCurrentAnim()
{
	return m_anim;
}
