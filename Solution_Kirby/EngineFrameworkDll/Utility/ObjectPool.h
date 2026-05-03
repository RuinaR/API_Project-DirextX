#pragma once
#include <iostream>
#define OBJNUM 10

template<typename T>
class SharedPointer;

template <typename T>
class ObjectPool
{
private:
    std::vector<SharedPointer<T>> m_freeObjects; // 지금 비어 있는 오브젝트 목록
    std::vector<SharedPointer<T>> m_usedObjects; // 현재 사용 중인 오브젝트 목록
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

    // 오브젝트를 하나 꺼내 쓴다.
    SharedPointer<T> AcquireObject()
    {
        if (m_freeObjects.empty())
        {
            // 남아 있는 오브젝트가 없으면 새로 만든다.
            SharedPointer<T> obj(new T());
            obj.SetPool(this);
            m_usedObjects.push_back(obj);
            return obj;
        }
        else
        {
            // 남아 있는 오브젝트가 있으면 그걸 다시 쓴다.
            SharedPointer<T> obj = m_freeObjects.back();
            m_freeObjects.pop_back();
            m_usedObjects.push_back(obj);
            return obj;
        }
    }

    // 사용이 끝난 오브젝트를 다시 pool로 돌려보낸다.
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
