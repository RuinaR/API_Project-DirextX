#pragma once
#include <Windows.h>
class Timer
{
public:
    Timer()
    {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        totalDeltaTime = 0.0; // 총 누적 시간 초기화
    }

    void tick()
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        LONGLONG elapsedTime = currentTime.QuadPart - lastTime.QuadPart;
        deltaTime = double(elapsedTime) / frequency.QuadPart;
        lastTime = currentTime;
        totalDeltaTime += deltaTime; // deltaTime을 계속 누적
    }

    double getDeltaTime() const
    {
        return deltaTime;
    }

    double getTotalDeltaTime() const
    {
        return totalDeltaTime; // 누적된 deltaTime을 반환
    }

    void resetTotalDeltaTime()
    {
        totalDeltaTime = 0.0; // 누적 델타 시간을 리셋
    }

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    double deltaTime = 0.0; // 현재 프레임과 이전 프레임 사이의 시간 차이
    double totalDeltaTime = 0.0; // 누적된 deltaTime
};