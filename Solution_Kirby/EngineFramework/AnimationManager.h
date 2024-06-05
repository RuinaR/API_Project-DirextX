#pragma once

#include "AnimationRender.h"

static class AnimationManager
{
private:
	static bool IsBitmapFile(const wstring& filename);
public:
	static Animation LoadAnimation(const wstring& folderName, float time);
	static void ReleaseAnimation(Animation& anim);
	static IDirect3DTexture9* LoadTexture(const wstring& folderName);
	static void ReleaseTexture(IDirect3DTexture9* tex);
};

