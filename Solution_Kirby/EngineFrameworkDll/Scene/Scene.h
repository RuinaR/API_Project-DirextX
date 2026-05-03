#pragma once
#include "../EngineFrameworkAPI.h"
class ENGINEFRAMEWORK_API Scene abstract
{
protected:
	float m_timeScale = 1.0f;

public:
	virtual void Init() abstract;
	virtual void Release() abstract;
	virtual void Start() abstract;
	virtual const char* GetSceneName() const abstract;
	virtual void BuildInitialSceneObjects() abstract;
	virtual bool ShouldUseSceneData() const { return true; }
	virtual bool ShouldSaveInitialSceneData() const { return true; }
	float GetTimeScale() const { return m_timeScale; }
	void SetTimeScale(float timeScale) { m_timeScale = timeScale < 0.0f ? 0.0f : timeScale; }

};
