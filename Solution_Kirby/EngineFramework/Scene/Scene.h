#pragma once
class Scene abstract
{
public:
	virtual void Init() abstract;
	virtual void Release() abstract;
	virtual void Start() abstract;
	virtual const char* GetSceneName() const abstract;
	virtual void BuildInitialSceneObjects() abstract;
	virtual bool ShouldUseSceneData() const { return true; }
	virtual bool ShouldSaveInitialSceneData() const { return true; }

};
