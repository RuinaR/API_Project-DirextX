#pragma once
#include <Windows.h>
class Timer
{
public:
    Timer()
    {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        totalDeltaTime = 0.0; // 지금까지 누적된 시간을 0으로 맞춘다.
    }

    void tick()
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        LONGLONG elapsedTime = currentTime.QuadPart - lastTime.QuadPart;
        deltaTime = double(elapsedTime) / frequency.QuadPart;
        lastTime = currentTime;
        totalDeltaTime += deltaTime; // 프레임 시간을 계속 더해 간다.
    }

    double getDeltaTime() const
    {
        return deltaTime;
    }

    double getTotalDeltaTime() const
    {
        return totalDeltaTime; // 지금까지 누적된 시간을 돌려준다.
    }

    void resetTotalDeltaTime()
    {
        totalDeltaTime = 0.0; // 누적 시간을 다시 0으로 만든다.
    }

    void Resync()
    {
        QueryPerformanceCounter(&lastTime);
        deltaTime = 0.0;
        totalDeltaTime = 0.0;
    }

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    double deltaTime = 0.0; // 현재 프레임과 이전 프레임 사이 시간
    double totalDeltaTime = 0.0; // 지금까지 더해 온 전체 시간
};
