#pragma once

#include "AnimationRender.h"

static class AnimationManager
{
private:
	static bool IsImageFile(const wstring& filename);
public:
	static Animation LoadAnimation(const wstring& folderName, float time);
	static IDirect3DTexture9* LoadTexture(const string& folderName);
};

