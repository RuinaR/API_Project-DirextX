// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
//#include "pch.h"
#include <Windows.h>

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // DLL 모듈 핸들
    DWORD fdwReason,     // 호출 사유
    LPVOID lpReserved)   // 예약 인자
{
    // 호출 사유에 따라 필요한 처리를 수행한다.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // 새 프로세스에 대해 한 번 초기화한다.
        // FALSE를 반환하면 DLL 로드가 실패한다.
        break;

    case DLL_THREAD_ATTACH:
        // 스레드별 초기화가 필요하면 여기서 처리한다.
        break;

    case DLL_THREAD_DETACH:
        // 스레드별 정리가 필요하면 여기서 처리한다.
        break;

    case DLL_PROCESS_DETACH:
        // DLL 해제 시 필요한 정리를 수행한다.
        break;
    }
    return TRUE;  // DLL_PROCESS_ATTACH 성공
}
