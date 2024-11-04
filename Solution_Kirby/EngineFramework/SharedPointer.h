#pragma once

template<typename T>
class ObjectPool;

template<typename T>
class SharedPointer
{
private:
    T* m_ptr;
    int* m_refCnt;
    ObjectPool<T>* m_myPool;

    void release()
    {
        if (!m_myPool && m_refCnt && --(*m_refCnt) == 0)
        {
            delete m_ptr;
            delete m_refCnt;
            m_ptr = nullptr;
            m_refCnt = nullptr;
            m_myPool = nullptr;
        }
        else if (m_myPool && (*m_refCnt) == 1)
        {
            m_myPool->ReleaseObject(this);
        }
    }

public:

    SharedPointer(const SharedPointer<T>& other)
        : m_ptr(other.m_ptr), m_refCnt(other.m_refCnt), m_myPool(other.m_myPool)
    {
        if (m_refCnt)
        {
            ++(*m_refCnt);
        }
    }

    SharedPointer(T* ptr)
        : m_ptr(ptr), m_refCnt(ptr ? new int(1) : nullptr), m_myPool(nullptr) {}

    SharedPointer(SharedPointer<T>&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)),
        m_refCnt(std::exchange(other.m_refCnt, nullptr)),
        m_myPool(std::exchange(other.m_myPool, nullptr)) {}

    SharedPointer<T>& operator=(const SharedPointer<T>& other)
    {
        if (this != &other)
        {
            release();
            m_ptr = other.m_ptr;
            m_refCnt = other.m_refCnt;
            m_myPool = other.m_myPool;
            if (m_refCnt)
            {
                ++(*m_refCnt);
            }
        }
        return *this;
    }

    SharedPointer<T>& operator=(SharedPointer<T>&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_ptr = std::exchange(other.m_ptr, nullptr);
            m_refCnt = std::exchange(other.m_refCnt, nullptr);
            m_myPool = std::exchange(other.m_myPool, nullptr);
        }
        return *this;
    }

    SharedPointer<T>& operator=(T* ptr)
    {
        if (m_ptr != ptr)
        {
            release();
            m_ptr = ptr;
            m_myPool = nullptr;
            m_refCnt = ptr ? new int(1) : nullptr;
        }
        return *this;
    }

    bool operator==(const SharedPointer<T> other)
    {
        if (m_ptr == other.m_ptr)
        {
            return true;
        }
        return false;
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

    T* Get() const
    {
        return m_ptr;
    }

    int use_count() const
    {
        return m_refCnt ? *m_refCnt : 0;
    }

    void SetPool(ObjectPool<T>* pool)
    {
        m_myPool = pool;
    }
};
