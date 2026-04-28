#pragma once
#include <iostream>
#define OBJNUM 10

template<typename T>
class SharedPointer;

template <typename T>
class ObjectPool
{
private:
    std::vector<SharedPointer<T>> m_freeObjects; // 사용되지 않는 오브젝트
    std::vector<SharedPointer<T>> m_usedObjects; // 사용 중인 오브젝트
public:
    // 생성자
    ObjectPool(int initialSize = OBJNUM)
    {
        for (int i = 0; i < initialSize; ++i)
        {
            SharedPointer<T> obj(new T());
            obj.SetPool(this);
            m_freeObjects.push_back(obj);
        }
    }

    // 소멸자
    ~ObjectPool()
    {
    }

    void ShowDebug()
    {
        std::cout << "freeObjects : " << m_freeObjects.size() << std::endl;
        std::cout << "usedObjects : " << m_usedObjects.size() << std::endl << std::endl;
    }

    // 오브젝트 할당 함수
    SharedPointer<T> AcquireObject()
    {
        if (m_freeObjects.empty())
        {
            // 사용 가능한 오브젝트가 없으면 새 오브젝트 생성
            SharedPointer<T> obj(new T());
            obj.SetPool(this);
            m_usedObjects.push_back(obj);
            return obj;
        }
        else
        {
            // 사용 가능한 오브젝트가 있으면 반환
            SharedPointer<T> obj = m_freeObjects.back();
            m_freeObjects.pop_back();
            m_usedObjects.push_back(obj);
            return obj;
        }
    }

    // 오브젝트 해제 함수 (사용 종료 시)
    void ReleaseObject(SharedPointer<T> obj)
    {
        auto it = find(m_usedObjects.begin(), m_usedObjects.end(), obj);
        if (it != m_usedObjects.end())
        {
            m_usedObjects.erase(it);
            m_freeObjects.push_back(obj);
        }
    }

    void ReleaseObject(SharedPointer<T>* obj) 
    {
        auto  it = std::find_if(m_usedObjects.begin(), m_usedObjects.end(),
            [&obj](const SharedPointer<T>& o) 
            { 
                return o.Get() == obj->Get();
            }
        );
        if (it != m_usedObjects.end()) 
        {
            m_usedObjects.erase(it);
            m_freeObjects.push_back(*obj);
        }
    }
};