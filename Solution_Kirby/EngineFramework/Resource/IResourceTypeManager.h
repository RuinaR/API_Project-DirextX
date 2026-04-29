#pragma once

class IResourceTypeManager
{
public:
	virtual ~IResourceTypeManager() {}
	virtual void ReleaseAllResources() = 0;
};
