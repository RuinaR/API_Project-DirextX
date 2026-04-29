#include "pch.h"
#include "AnimationRender.h"
#include "Editor/EditorAssetField.h"
#include "ImageRender.h"
#include "GameObject.h"
#include "Resource/AssetDatabase.h"
#include "Resource/ResourceManager.h"
#include "SceneJsonUtility.h"
#include <algorithm>
#include <cctype>

namespace
{
	constexpr float kMinFrameDuration = 0.01f;

	std::string GetFileExtension(const std::string& fileName)
	{
		const size_t dotPosition = fileName.find_last_of('.');
		if (dotPosition == std::string::npos)
		{
			return "";
		}

		return fileName.substr(dotPosition);
	}

	bool IsAnimationFrameExtension(const std::string& extension)
	{
		std::string lowerExtension = extension;
		std::transform(lowerExtension.begin(), lowerExtension.end(), lowerExtension.begin(),
			[](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

		return lowerExtension == ".png"
			|| lowerExtension == ".jpg"
			|| lowerExtension == ".jpeg"
			|| lowerExtension == ".bmp"
			|| lowerExtension == ".dds";
	}

}

AnimationRender::AnimationRender(Animation anim)
	:Component(), m_isPlay(true), m_updateTimer(0.0f), m_curItr(), m_bitren(nullptr)
{
	m_anim = anim;
	SetFrameDuration(m_anim.time > 0.0f ? m_anim.time : m_frameDuration);
}

ImageRender* AnimationRender::ResolveImageRender()
{
	if (m_gameObj == nullptr)
	{
		m_bitren = nullptr;
		return nullptr;
	}

	m_bitren = m_gameObj->GetComponent<ImageRender>();
	return m_bitren;
}

void AnimationRender::Initialize()
{
	m_released = false;
	m_reloadRequested = !m_animationFolderKey.empty();
	m_bitren = ResolveImageRender();
	if (m_bitren == nullptr)
	{
		m_bitren = new ImageRender(nullptr);
		m_gameObj->AddComponent(m_bitren);
	}
}

void AnimationRender::Release()
{
	if (m_released)
	{
		return;
	}

	m_released = true;
	if (ResolveImageRender() != nullptr)
	{
		m_bitren->ChangeTexture(nullptr);
	}
	m_bitren = nullptr;
	m_reloadRequested = false;
	m_clip.frames.clear();
	m_runtimeFrames.clear();
	m_runtimeFrameIndex = 0;
	m_anim.textures.clear();
	m_curItr = m_anim.textures.end();
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
	if (ResolveImageRender() != nullptr)
	{
		m_bitren->ChangeTexture(*m_curItr);
	}
}

void AnimationRender::Start()
{
	if (m_reloadRequested)
	{
		ReloadFromFolder();
	}

	m_isPlay = m_playOnStart;

	if (!m_runtimeFrames.empty())
	{
		m_runtimeFrameIndex = 0;
		m_updateTimer = 0.0f;
		if (ResolveImageRender() != nullptr && m_runtimeFrames[m_runtimeFrameIndex].texture != nullptr)
		{
			m_bitren->ChangeTexture(
				m_runtimeFrames[m_runtimeFrameIndex].texture,
				m_runtimeFrames[m_runtimeFrameIndex].textureKey);
		}
		return;
	}

	if (!m_anim.textures.empty())
	{
		m_curItr = m_anim.textures.begin();
	}
}

void AnimationRender::Update()
{
	if (m_reloadRequested)
	{
		ReloadFromFolder();
	}

	if (!m_runtimeFrames.empty())
	{
		if (!m_isPlay)
			return;

		if (m_runtimeFrameIndex >= m_runtimeFrames.size())
		{
			m_runtimeFrameIndex = 0;
		}

		const float frameDuration = m_runtimeFrames[m_runtimeFrameIndex].duration > 0.0f
			? m_runtimeFrames[m_runtimeFrameIndex].duration
			: m_frameDuration;

		m_updateTimer += MainFrame::GetInstance()->DeltaTime();
		if (m_updateTimer < frameDuration)
			return;

		if ((m_isOneTime || !m_loop) && m_runtimeFrameIndex + 1 >= m_runtimeFrames.size())
			return;

		m_updateTimer = 0.0f;
		m_runtimeFrameIndex++;
		if (m_runtimeFrameIndex >= m_runtimeFrames.size())
		{
			m_runtimeFrameIndex = 0;
		}

		if (ResolveImageRender() != nullptr && m_runtimeFrames[m_runtimeFrameIndex].texture != nullptr)
		{
			m_bitren->ChangeTexture(
				m_runtimeFrames[m_runtimeFrameIndex].texture,
				m_runtimeFrames[m_runtimeFrameIndex].textureKey);
		}
		return;
	}

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

	if ((m_isOneTime || !m_loop) && m_curItr == --m_anim.textures.end())
		return;
	m_updateTimer = 0.0f;
	m_curItr++;
	if (m_curItr == m_anim.textures.end())
	{
		if (!m_loop)
		{
			m_curItr = --m_anim.textures.end();
			return;
		}
		m_curItr = m_anim.textures.begin();
	}

	if (ResolveImageRender() != nullptr)
	{
		m_bitren->ChangeTexture(*m_curItr);
	}
}

void AnimationRender::SetPlay(bool play)
{
	m_isPlay = play;
}

void AnimationRender::SetOneTime(bool b)
{
	m_isOneTime = b;
	m_loop = !b;
}

bool AnimationRender::IsFinishAnim()
{
	if (!m_runtimeFrames.empty())
	{
		return m_isOneTime && m_runtimeFrameIndex + 1 >= m_runtimeFrames.size();
	}

	if (m_anim.textures.empty())
	{
		return false;
	}

	if (m_isOneTime && m_curItr == --m_anim.textures.end())
		return true;

	return false;
}

const Animation& AnimationRender::GetCurrentAnim()
{
	return m_anim;
}

const std::string& AnimationRender::GetAnimationFolderKey() const
{
	return m_animationFolderKey;
}

void AnimationRender::SetAnimationFolderKey(const std::string& animationFolderKey)
{
	if (m_animationFolderKey == animationFolderKey)
	{
		return;
	}

	m_animationFolderKey = animationFolderKey;
	RequestReloadFromFolder();
}

void AnimationRender::ReloadFromFolder()
{
	m_reloadRequested = false;
	RebuildClipFromFolder();
	RebuildRuntimeFrames();
}

void AnimationRender::RequestReloadFromFolder()
{
	m_reloadRequested = true;
	m_clip.animationFolderKey = m_animationFolderKey;
	m_clip.frames.clear();
	m_runtimeFrames.clear();
	m_runtimeFrameIndex = 0;
	m_updateTimer = 0.0f;
	if (ResolveImageRender() != nullptr)
	{
		m_bitren->ChangeTexture(nullptr);
	}
}

void AnimationRender::RebuildClipFromFolder()
{
	m_clip.animationFolderKey = m_animationFolderKey;
	m_clip.defaultFrameDuration = m_frameDuration;
	m_clip.loop = m_loop;
	m_clip.playOnStart = m_playOnStart;
	m_clip.frames.clear();
	m_runtimeFrames.clear();
	m_runtimeFrameIndex = 0;
	m_updateTimer = 0.0f;

	if (m_animationFolderKey.empty())
	{
		return;
	}

	AssetDatabase* assetDatabase = AssetDatabase::GetInstance();
	if (assetDatabase == nullptr)
	{
		return;
	}

	const AssetInfo* assetInfo = assetDatabase->FindByKey(m_animationFolderKey);
	if (assetInfo == nullptr || assetInfo->type != AssetType::Animation || assetInfo->path.empty())
	{
		return;
	}

	const DWORD attributes = GetFileAttributesA(assetInfo->path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return;
	}

	std::string searchPath = assetInfo->path;
	if (!searchPath.empty() && searchPath.back() != '\\' && searchPath.back() != '/')
	{
		searchPath += "\\";
	}
	searchPath += "*";

	std::vector<std::string> frameFiles;
	WIN32_FIND_DATAA findData = {};
	HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			continue;
		}

		std::string fileName = findData.cFileName;
		if (IsAnimationFrameExtension(GetFileExtension(fileName)))
		{
			frameFiles.push_back(fileName);
		}
	} while (FindNextFileA(findHandle, &findData) != 0);

	FindClose(findHandle);
	std::sort(frameFiles.begin(), frameFiles.end());

	for (const std::string& frameFile : frameFiles)
	{
		AnimationFrameData frame;
		frame.textureKey = m_animationFolderKey + "/" + frameFile;
		frame.duration = m_frameDuration;
		m_clip.frames.push_back(frame);
	}
}

void AnimationRender::RebuildRuntimeFrames()
{
	m_runtimeFrames.clear();
	m_runtimeFrameIndex = 0;

	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (resourceManager == nullptr)
	{
		return;
	}

	for (const AnimationFrameData& frameData : m_clip.frames)
	{
		if (frameData.textureKey.empty())
		{
			continue;
		}

		IDirect3DTexture9* texture = resourceManager->GetTexture(frameData.textureKey);
		if (texture == nullptr)
		{
			continue;
		}

		AnimationRuntimeFrame runtimeFrame;
		runtimeFrame.texture = texture;
		runtimeFrame.textureKey = frameData.textureKey;
		runtimeFrame.duration = frameData.duration > 0.0f ? frameData.duration : m_frameDuration;
		m_runtimeFrames.push_back(runtimeFrame);
	}

	if (ResolveImageRender() != nullptr && !m_runtimeFrames.empty() && m_runtimeFrames[0].texture != nullptr)
	{
		m_bitren->ChangeTexture(m_runtimeFrames[0].texture, m_runtimeFrames[0].textureKey);
	}
}

const AnimationClipData& AnimationRender::GetClipData() const
{
	return m_clip;
}

const std::vector<AnimationRuntimeFrame>& AnimationRender::GetRuntimeFrames() const
{
	return m_runtimeFrames;
}

float AnimationRender::GetFrameDuration() const
{
	return m_frameDuration;
}

void AnimationRender::SetFrameDuration(float frameDuration)
{
	if (frameDuration < kMinFrameDuration)
	{
		frameDuration = kMinFrameDuration;
	}

	m_frameDuration = frameDuration;
	m_anim.time = frameDuration;
	m_clip.defaultFrameDuration = frameDuration;
	for (AnimationFrameData& frame : m_clip.frames)
	{
		frame.duration = frameDuration;
	}
	for (AnimationRuntimeFrame& frame : m_runtimeFrames)
	{
		frame.duration = frameDuration;
	}
}

bool AnimationRender::GetLoop() const
{
	return m_loop;
}

void AnimationRender::SetLoop(bool loop)
{
	m_loop = loop;
	m_isOneTime = !loop;
	m_clip.loop = loop;
}

bool AnimationRender::GetPlayOnStart() const
{
	return m_playOnStart;
}

void AnimationRender::SetPlayOnStart(bool playOnStart)
{
	m_playOnStart = playOnStart;
	m_clip.playOnStart = playOnStart;
}

const char* AnimationRender::GetInspectorName() const
{
	return "AnimationRender";
}

void AnimationRender::DrawInspector()
{
	std::string animationFolderKey = m_animationFolderKey;
	if (EditorAssetField::Draw("Animation Folder", AssetType::Animation, animationFolderKey))
	{
		SetAnimationFolderKey(animationFolderKey);
	}

	ImGui::Text("Texture Count: %d", static_cast<int>(m_anim.textures.size()));
	ImGui::Text("Runtime Frame Count: %d", static_cast<int>(m_runtimeFrames.size()));
	if (ImGui::Button("Reload Animation"))
	{
		RequestReloadFromFolder();
	}
	ImGui::Text("Animation Folder Key: %s", m_animationFolderKey.empty() ? "(none)" : m_animationFolderKey.c_str());
	float frameDuration = m_frameDuration;
	if (ImGui::InputFloat("Frame Duration", &frameDuration, 0.01f, 0.1f, "%.3f"))
	{
		SetFrameDuration(frameDuration);
	}
	bool playOnStart = m_playOnStart;
	if (ImGui::Checkbox("Play On Start", &playOnStart))
	{
		SetPlayOnStart(playOnStart);
	}
	ImGui::Text("Source Path: %s", m_anim.sourcePath.empty() ? "(none)" : m_anim.sourcePath.c_str());
	ImGui::Text("Update Timer: %.3f", m_updateTimer);
	ImGui::Checkbox("Play", &m_isPlay);
	bool oneTime = m_isOneTime;
	if (ImGui::Checkbox("One Shot", &oneTime))
	{
		SetOneTime(oneTime);
	}
	ImGui::SameLine();
	bool loop = m_loop;
	if (ImGui::Checkbox("Loop", &loop))
	{
		SetLoop(loop);
	}
	ImGui::Text("ImageRender: %s", ResolveImageRender() ? "Linked" : "None");
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
	oss << "\"animationFolderKey\": \"" << SceneJson::EscapeString(m_animationFolderKey) << "\", ";
	oss << "\"frameDuration\": " << m_frameDuration << ", ";
	oss << "\"loop\": " << (m_loop ? "true" : "false") << ", ";
	oss << "\"playOnStart\": " << (m_playOnStart ? "true" : "false") << ", ";
	oss << "\"sourcePath\": \"" << SceneJson::EscapeString(m_anim.sourcePath) << "\"";
	oss << " }";
	return oss.str();
}

bool AnimationRender::Deserialize(const std::string& componentJson)
{
	SceneJson::ReadBool(componentJson, "play", m_isPlay);
	SceneJson::ReadBool(componentJson, "oneTime", m_isOneTime);
	m_loop = !m_isOneTime;
	SceneJson::ReadBool(componentJson, "loop", m_loop);
	m_isOneTime = !m_loop;
	SceneJson::ReadBool(componentJson, "playOnStart", m_playOnStart);

	float frameTime = m_frameDuration;
	if (SceneJson::ReadFloat(componentJson, "frameTime", frameTime))
	{
		SetFrameDuration(frameTime);
	}
	SceneJson::ReadFloat(componentJson, "frameDuration", frameTime);
	SetFrameDuration(frameTime);

	std::string animationFolderKey;
	if (SceneJson::ReadString(componentJson, "animationFolderKey", animationFolderKey))
	{
		SetAnimationFolderKey(animationFolderKey);
	}

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
	if (ResolveImageRender() != nullptr && !m_anim.textures.empty())
	{
		m_bitren->ChangeTexture(*m_curItr);
	}
	return true;
}
