#pragma once
#include <atomic>
#include <utility>

template<typename T>
class SharedPointer
{
public:
    explicit SharedPointer(T* ptr = nullptr)
        : m_ptr(ptr), m_refCnt(ptr ? new std::atomic<int>(1) : nullptr) {}

    SharedPointer(const SharedPointer<T>& other)
        : m_ptr(other.m_ptr), m_refCnt(other.m_refCnt)
    {
        if (m_refCnt) {
            ++(*m_refCnt);
        }
    }

    SharedPointer(SharedPointer<T>&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)),
        m_refCnt(std::exchange(other.m_refCnt, nullptr)) {}

    SharedPointer<T>& operator=(const SharedPointer<T>& other)
    {
        if (this != &other) {
            release();
            m_ptr = other.m_ptr;
            m_refCnt = other.m_refCnt;
            if (m_refCnt) {
                ++(*m_refCnt);
            }
        }
        return *this;
    }

    SharedPointer<T>& operator=(SharedPointer<T>&& other) noexcept
    {
        if (this != &other) {
            release();
            m_ptr = std::exchange(other.m_ptr, nullptr);
            m_refCnt = std::exchange(other.m_refCnt, nullptr);
        }
        return *this;
    }

    SharedPointer<T>& operator=(T* ptr)
    {
        if (m_ptr != ptr) {
            release();
            m_ptr = ptr;
            m_refCnt = ptr ? new std::atomic<int>(1) : nullptr;
        }
        return *this;
    }

    ~SharedPointer()
    {
        release();
    }

    T& operator*() const
    {
        return *m_ptr;
    }

    T* operator->() const
    {
        return m_ptr;
    }

    T* get() const
    {
        return m_ptr;
    }

    int use_count() const
    {
        return m_refCnt ? *m_refCnt : 0;
    }

private:
    void release()
    {
        if (m_refCnt && --(*m_refCnt) == 0) 
        {
            delete m_ptr;
            delete m_refCnt;
        }
        m_ptr = nullptr;
        m_refCnt = nullptr;
    }

    T* m_ptr;
    std::atomic<int>* m_refCnt;
};