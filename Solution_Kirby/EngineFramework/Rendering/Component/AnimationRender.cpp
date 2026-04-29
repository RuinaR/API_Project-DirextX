#include "pch.h"
#include "AnimationRender.h"
#include "ImageRender.h"
#include "GameObject.h"
#include "Resource/ResourceManager.h"
#include "SceneJsonUtility.h"

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

const char* AnimationRender::GetInspectorName() const
{
	return "AnimationRender";
}

void AnimationRender::DrawInspector()
{
	ImGui::Text("Texture Count: %d", static_cast<int>(m_anim.textures.size()));
	ImGui::Text("Frame Time: %.3f", m_anim.time);
	ImGui::Text("Source Path: %s", m_anim.sourcePath.empty() ? "(none)" : m_anim.sourcePath.c_str());
	ImGui::Text("Update Timer: %.3f", m_updateTimer);
	ImGui::Checkbox("Play", &m_isPlay);
	ImGui::Checkbox("One Time", &m_isOneTime);
	ImGui::Text("ImageRender: %s", m_bitren ? "Linked" : "None");
}

const char* AnimationRender::GetSerializableType() const
{
	return "AnimationRender";
}

std::string AnimationRender::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"play\": " << (m_isPlay ? "true" : "false") << ", ";
	oss << "\"oneTime\": " << (m_isOneTime ? "true" : "false") << ", ";
	oss << "\"frameTime\": " << m_anim.time << ", ";
	oss << "\"sourcePath\": \"" << SceneJson::EscapeString(m_anim.sourcePath) << "\"";
	oss << " }";
	return oss.str();
}

bool AnimationRender::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "play", m_isPlay);
	SceneJson::ReadBool(componentJson, "oneTime", m_isOneTime);
	SceneJson::ReadFloat(componentJson, "frameTime", m_anim.time);

	std::string sourcePath;
	if (SceneJson::ReadString(componentJson, "sourcePath", sourcePath) && !sourcePath.empty())
	{
		m_anim = ResourceManager::GetInstance()->GetAnimation(ConvertToWideString(sourcePath), m_anim.time);
		m_anim.sourcePath = sourcePath;
	}
	else
	{
		std::cout << "AnimationRender deserialize: sourcePath missing. Animation frames are not restored." << std::endl;
	}

	m_curItr = m_anim.textures.begin();
	if (m_bitren != nullptr && !m_anim.textures.empty())
	{
		m_bitren->ChangeTexture(*m_curItr);
	}
	return true;
}
