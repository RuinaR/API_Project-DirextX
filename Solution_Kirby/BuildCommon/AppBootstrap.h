#pragma once

#include <wtypes.h>

#include "../include/AppLaunchDesc.h"

bool BootstrapMainFrame(HINSTANCE hInstance, const AppLaunchDesc& launchDesc);
bool UpdateMainFrame();
void ShutdownMainFrame();
