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

struct Animation
{
	vector<IDirect3DTexture9*> textures;
	float time = 0.0f;
	int identity = 0;
	std::string sourcePath;
};
