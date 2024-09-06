#pragma once
#include "FbxTool.h"
class FBXRender :public Component
{
private:
	FbxTool m_tool;
	std::string m_name;
public:
	FBXRender(std::string name);
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
};

