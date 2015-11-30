/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
/** 
 *****************************************************************
 * @file <CustomSinkLinkList.h> 
 *   
 *
 *****************************************************************
 */
#ifndef CUSTOMSINKLINKLIST_H_
#define CUSTOMSINKLINKLIST_H_

template<class T>
struct NoOp
{
#ifdef _WIN32
    void operator()(T& /*t*/)
#else
    void operator()(T& t)
#endif
    {
    }
};

template<class T>
class List
{
protected:

    struct Node
    {
        Node *prev;
        Node *next;
        T item;

        Node() :
            prev(nullptr), next(nullptr)
        {
        }

        Node(T item) :
            prev(nullptr), next(nullptr)
        {
            this->item = item;
        }

        T Item() const
        {
            return item;
        }
    };

public:

    class POSITION
    {
        friend class List<T> ;

    public:
        POSITION() :
            pNode(nullptr)
        {
        }

        bool operator==(const POSITION &p) const
        {
            return pNode == p.pNode;
        }

        bool operator!=(const POSITION &p) const
        {
            return pNode != p.pNode;
        }

    private:
        const Node *pNode;

        POSITION(Node *p) :
            pNode(p)
        {
        }
    };

protected:
    Node m_anchor;
    DWORD m_count;

    Node* Front() const
    {
        return m_anchor.next;
    }

    Node* Back() const
    {
        return m_anchor.prev;
    }

    virtual HRESULT InsertAfter(T item, Node *pBefore)
    {
        if (pBefore == nullptr)
        {
            return E_POINTER;
        }

        Node *pNode = new Node(item);
        if (pNode == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        Node *pAfter = pBefore->next;

        pBefore->next = pNode;
        pAfter->prev = pNode;

        pNode->prev = pBefore;
        pNode->next = pAfter;

        m_count++;

        return S_OK;
    }

    virtual HRESULT GetItem(const Node *pNode, T* ppItem)
    {
        if (pNode == nullptr || ppItem == nullptr)
        {
            return E_POINTER;
        }

        *ppItem = pNode->item;
        return S_OK;
    }

    virtual HRESULT RemoveItem(Node *pNode, T *ppItem)
    {
        if (pNode == nullptr)
        {
            return E_POINTER;
        }

        assert(pNode != &m_anchor);
        if (pNode == &m_anchor)
        {
            return E_INVALIDARG;
        }

        T item;

        pNode->next->prev = pNode->prev;

        pNode->prev->next = pNode->next;

        item = pNode->item;
        delete pNode;

        m_count--;

        if (ppItem)
        {
            *ppItem = item;
        }

        return S_OK;
    }

public:

    List()
    {
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
    }

    virtual ~List()
    {
        Clear();
    }

    HRESULT InsertBack(T item)
    {
        return InsertAfter(item, m_anchor.prev);
    }

    HRESULT InsertFront(T item)
    {
        return InsertAfter(item, &m_anchor);
    }

    HRESULT InsertPos(POSITION pos, T item)
    {
        if (pos.pNode == nullptr)
        {
            return InsertBack(item);
        }

        return InsertAfter(item, pos.pNode->prev);
    }

    HRESULT RemoveBack(T *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Back(), ppItem);
        }
    }

    HRESULT RemoveFront(T *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Front(), ppItem);
        }
    }

    HRESULT GetBack(T *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Back(), ppItem);
        }
    }

    HRESULT GetFront(T *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Front(), ppItem);
        }
    }

    DWORD GetCount() const
    {
        return m_count;
    }

    bool IsEmpty() const
    {
        return (GetCount() == 0);
    }

    template<class FN>
    void Clear(FN& clear_fn)
    {
        Node *n = m_anchor.next;

        while (n != &m_anchor)
        {
            clear_fn(n->item);

            Node *tmp = n->next;
            delete n;
            n = tmp;
        }

        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
    }

    virtual void Clear()
    {
        NoOp<T> clearOp;
        Clear<> (clearOp);
    }

    POSITION FrontPosition()
    {
        if (IsEmpty())
        {
            return POSITION(nullptr);
        }
        else
        {
            return POSITION(Front());
        }
    }

    POSITION EndPosition() const
    {
        return POSITION();
    }

    HRESULT GetItemPos(POSITION pos, T *ppItem)
    {
        if (pos.pNode)
        {
            return GetItem(pos.pNode, ppItem);
        }
        else
        {
            return E_FAIL;
        }
    }

    POSITION Next(const POSITION pos)
    {
        if (pos.pNode && (pos.pNode->next != &m_anchor))
        {
            return POSITION(pos.pNode->next);
        }
        else
        {
            return POSITION(nullptr);
        }
    }

    HRESULT Remove(POSITION& pos, T *ppItem)
    {
        if (pos.pNode)
        {
            Node *pNode = const_cast<Node*> (pos.pNode);

            pos = POSITION();

            return RemoveItem(pNode, ppItem);
        }
        else
        {
            return E_INVALIDARG;
        }
    }

};
class ComAutoRelease
{
public:
    void operator()(IUnknown *p)
    {
        if (p)
        {
            p->Release();
        }
    }
};
class MemDelete
{
public:
    void operator()(void *p)
    {
        if (p)
        {
            delete p;
        }
    }
};

template<class T, bool NULLABLE = FALSE>
class ComPtrList: public List<T*>
{
public:

    typedef T* Ptr;

    void Clear()
    {
        ComAutoRelease car;
        List<Ptr>::Clear(car);
    }

    ~ComPtrList()
    {
        Clear();
    }

protected:
    HRESULT InsertAfter(Ptr item, Node *pBefore)
    {
        if (item == nullptr && !NULLABLE)
        {
            return E_POINTER;
        }

        if (item)
        {
            item->AddRef();
        }

        HRESULT hr = List<Ptr>::InsertAfter(item, pBefore);
        if (FAILED(hr) && item != nullptr)
        {
            item->Release();
        }
        return hr;
    }

    HRESULT GetItem(const Node *pNode, Ptr* ppItem)
    {
        Ptr pItem = nullptr;

        HRESULT hr = List<Ptr>::GetItem(pNode, &pItem);
        if (SUCCEEDED(hr))
        {
            assert(pItem || NULLABLE);
            if (pItem)
            {
                *ppItem = pItem;
                (*ppItem)->AddRef();
            }
        }
        return hr;
    }

    HRESULT RemoveItem(Node *pNode, Ptr *ppItem)
    {
        Ptr pItem = nullptr;

        HRESULT hr = List<Ptr>::RemoveItem(pNode, &pItem);

        if (SUCCEEDED(hr))
        {
            assert(pItem || NULLABLE);
            if (ppItem && pItem)
            {
                *ppItem = pItem;
                (*ppItem)->AddRef();
            }

            if (pItem)
            {
                pItem->Release();
                pItem = nullptr;
            }
        }

        return hr;
    }
};
#endif //CUSTOMSINKLINKLIST_H_
