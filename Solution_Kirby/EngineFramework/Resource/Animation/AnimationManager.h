#pragma once

#include "AnimationRender.h"

static class AnimationManager
{
private:
	static bool IsImageFile(const wstring& filename);
public:
	static Animation LoadAnimation(const wstring& folderName, float time);
	static IDirect3DTexture9* LoadTexture(const string& path);
	static void LoadTexture(const string& path, std::function<void(IDirect3DTexture9*)> func);
	static void LoadTexture(const string& path, ImageRender* ir);

};

