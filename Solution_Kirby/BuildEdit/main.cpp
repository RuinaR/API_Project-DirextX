
#include "BuildLoopCls.h"
#include <Windows.h>

BuildLoopCls g_BuildLoopCls;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ���⿡ �ڵ带 �Է��մϴ�.
    g_BuildLoopCls.Init(hInstance);

    while (true)
    {
        if (!g_BuildLoopCls.Update())
            break;
    }

    g_BuildLoopCls.Release();
    return 0;
}