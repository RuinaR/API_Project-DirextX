#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <d3dx9.h>

using namespace std;
struct Vector2D
{
	double x = 0.0f;
	double y = 0.0f;
};

struct AnimationFrameData
{
	std::string textureKey;
	float duration = 0.1f;
};

struct AnimationClipData
{
	std::string animationFolderKey;
	std::vector<AnimationFrameData> frames;
	float defaultFrameDuration = 0.1f;
	bool loop = true;
	bool playOnStart = true;
};

struct AnimationRuntimeFrame
{
	IDirect3DTexture9* texture = nullptr;
	std::string textureKey;
	float duration = 0.1f;
};

// 예전에 쓰던 실행용 구조체다.
// 지금은 저장용으로 AnimationClipData를, 실행 중 프레임 데이터로는 AnimationRuntimeFrame을 쓴다.
struct Animation
{
	vector<IDirect3DTexture9*> textures;
	float time = 0.0f;
	int identity = 0;
	std::string sourcePath;
};
