// DLL이 시작되거나 종료될 때 먼저 들어오는 함수
//#include "pch.h"
#include <Windows.h>

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // 이 DLL 인스턴스
    DWORD fdwReason,     // 왜 호출됐는지 알려 주는 값
    LPVOID lpReserved)   // 지금은 쓰지 않는 예약 인자
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpReserved);

    // 필요하면 여기서 DLL 시작/종료 처리를 넣을 수 있다.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // 프로세스가 이 DLL을 처음 붙일 때 한 번 들어온다.
        break;

    case DLL_THREAD_ATTACH:
        // 스레드 시작 시 따로 처리할 일이 있으면 여기서 한다.
        break;

    case DLL_THREAD_DETACH:
        // 스레드 종료 시 따로 정리할 일이 있으면 여기서 한다.
        break;

    case DLL_PROCESS_DETACH:
        // 프로세스에서 DLL이 빠질 때 정리가 필요하면 여기서 한다.
        break;
    }
    return TRUE;  // DLL 사용을 계속 진행한다.
}
