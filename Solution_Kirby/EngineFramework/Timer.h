#pragma once
#include <Windows.h>
class Timer
{
public:
    Timer()
    {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        totalDeltaTime = 0.0; // �� ���� �ð� �ʱ�ȭ
    }

    void tick()
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        LONGLONG elapsedTime = currentTime.QuadPart - lastTime.QuadPart;
        deltaTime = double(elapsedTime) / frequency.QuadPart;
        lastTime = currentTime;
        totalDeltaTime += deltaTime; // deltaTime�� ��� ����
    }

    double getDeltaTime() const
    {
        return deltaTime;
    }

    double getTotalDeltaTime() const
    {
        return totalDeltaTime; // ������ deltaTime�� ��ȯ
    }

    void resetTotalDeltaTime()
    {
        totalDeltaTime = 0.0; // ���� ��Ÿ �ð��� ����
    }

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    double deltaTime = 0.0; // ���� �����Ӱ� ���� ������ ������ �ð� ����
    double totalDeltaTime = 0.0; // ������ deltaTime
};