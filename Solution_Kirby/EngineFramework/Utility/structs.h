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

// Deprecated legacy runtime structure.
// Use AnimationClipData for saved data and AnimationRuntimeFrame for runtime texture frames.
struct Animation
{
	vector<IDirect3DTexture9*> textures;
	float time = 0.0f;
	int identity = 0;
	std::string sourcePath;
};
