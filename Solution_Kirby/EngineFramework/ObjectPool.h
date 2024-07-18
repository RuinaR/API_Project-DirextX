#pragma once
#include <iostream>
#define OBJNUM 10

template<typename T>
class SharedPointer;

template <typename T>
class ObjectPool
{
private:
    std::vector<SharedPointer<T>> m_freeObjects; // ������ �ʴ� ������Ʈ
    std::vector<SharedPointer<T>> m_usedObjects; // ��� ���� ������Ʈ
public:
    // ������
    ObjectPool(int initialSize = OBJNUM)
    {
        for (int i = 0; i < initialSize; ++i)
        {
            SharedPointer<T> obj(new T());
            obj.SetPool(this);
            m_freeObjects.push_back(obj);
        }
    }

    // �Ҹ���
    ~ObjectPool()
    {
    }

    void ShowDebug()
    {
        std::cout << "freeObjects : " << m_freeObjects.size() << std::endl;
        std::cout << "usedObjects : " << m_usedObjects.size() << std::endl << std::endl;
    }

    // ������Ʈ �Ҵ� �Լ�
    SharedPointer<T> AcquireObject()
    {
        if (m_freeObjects.empty())
        {
            // ��� ������ ������Ʈ�� ������ �� ������Ʈ ����
            SharedPointer<T> obj(new T());
            obj.SetPool(this);
            m_usedObjects.push_back(obj);
            return obj;
        }
        else
        {
            // ��� ������ ������Ʈ�� ������ ��ȯ
            SharedPointer<T> obj = m_freeObjects.back();
            m_freeObjects.pop_back();
            m_usedObjects.push_back(obj);
            return obj;
        }
    }

    // ������Ʈ ���� �Լ� (��� ���� ��)
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