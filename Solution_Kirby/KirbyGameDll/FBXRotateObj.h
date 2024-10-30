#pragma once
#include <Component.h>
class FBXRotateObj :public Component
{
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
};

