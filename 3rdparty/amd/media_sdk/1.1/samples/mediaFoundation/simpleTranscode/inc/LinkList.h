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
 ********************************************************************************
 * @file <LinkList.h>
 *
 * @brief This file contains linked list implementation
 *
 ********************************************************************************
 */

#pragma once

template<class T>
struct NoOp
{
    void operator()(T& t)
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
    Node mAnchor;
    DWORD mCount;

    Node* front() const
    {
        return mAnchor.next;
    }

    Node* back() const
    {
        return mAnchor.prev;
    }

    virtual HRESULT insertAfter(T item, Node *pBefore)
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

        mCount++;

        return S_OK;
    }

    virtual HRESULT getItem(const Node *pNode, T* ppItem)
    {
        if (pNode == nullptr || ppItem == nullptr)
        {
            return E_POINTER;
        }

        *ppItem = pNode->item;
        return S_OK;
    }

    virtual HRESULT removeItem(Node *pNode, T *ppItem)
    {
        if (pNode == nullptr)
        {
            return E_POINTER;
        }

        assert(pNode != &mAnchor);
        if (pNode == &mAnchor)
        {
            return E_INVALIDARG;
        }

        T item;

        pNode->next->prev = pNode->prev;

        pNode->prev->next = pNode->next;

        item = pNode->item;
        delete pNode;

        mCount--;

        if (ppItem)
        {
            *ppItem = item;
        }

        return S_OK;
    }

public:

    List()
    {
        mAnchor.next = &mAnchor;
        mAnchor.prev = &mAnchor;

        mCount = 0;
    }

    virtual ~List()
    {
        clear();
    }

    HRESULT insertBack(T item)
    {
        return insertAfter(item, mAnchor.prev);
    }

    HRESULT insertFront(T item)
    {
        return insertAfter(item, &mAnchor);
    }

    HRESULT insertPos(POSITION pos, T item)
    {
        if (pos.pNode == nullptr)
        {
            return insertBack(item);
        }

        return insertAfter(item, pos.pNode->prev);
    }

    HRESULT removeBack(T *ppItem)
    {
        if (isEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return removeItem(back(), ppItem);
        }
    }

    HRESULT removeFront(T *ppItem)
    {
        if (isEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return removeItem(front(), ppItem);
        }
    }

    HRESULT getBack(T *ppItem)
    {
        if (isEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return getItem(back(), ppItem);
        }
    }

    HRESULT getFront(T *ppItem)
    {
        if (isEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return getItem(front(), ppItem);
        }
    }

    DWORD getCount() const
    {
        return mCount;
    }

    bool isEmpty() const
    {
        return (getCount() == 0);
    }

    template<class FN>
    void clear(FN& clear_fn)
    {
        Node *n = mAnchor.next;

        while (n != &mAnchor)
        {
            clear_fn(n->item);

            Node *tmp = n->next;
            delete n;
            n = tmp;
        }

        mAnchor.next = &mAnchor;
        mAnchor.prev = &mAnchor;

        mCount = 0;
    }

    virtual void clear()
    {
        NoOp<T> clearOp;
        clear<> (clearOp);
    }

    POSITION frontPosition()
    {
        if (isEmpty())
        {
            return POSITION(nullptr);
        }
        else
        {
            return POSITION(front());
        }
    }

    POSITION endPosition() const
    {
        return POSITION();
    }

    HRESULT getItemPos(POSITION pos, T *ppItem)
    {
        if (pos.pNode)
        {
            return getItem(pos.pNode, ppItem);
        }
        else
        {
            return E_FAIL;
        }
    }

    POSITION next(const POSITION pos)
    {
        if (pos.pNode && (pos.pNode->next != &mAnchor))
        {
            return POSITION(pos.pNode->next);
        }
        else
        {
            return POSITION(nullptr);
        }
    }

    HRESULT remove(POSITION& pos, T *ppItem)
    {
        if (pos.pNode)
        {
            Node *pNode = const_cast<Node*> (pos.pNode);

            pos = POSITION();

            return removeItem(pNode, ppItem);
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

    void clear()
    {
        ComAutoRelease car;
        List<Ptr>::clear(car);
    }

    ~ComPtrList()
    {
        clear();
    }

protected:
    HRESULT insertAfter(Ptr item, Node *pBefore)
    {
        if (item == nullptr && !NULLABLE)
        {
            return E_POINTER;
        }

        if (item)
        {
            item->AddRef();
        }

        HRESULT hr = List<Ptr>::insertAfter(item, pBefore);
        if (FAILED(hr) && item != nullptr)
        {
            item->Release();
        }
        return hr;
    }

    HRESULT getItem(const Node *pNode, Ptr* ppItem)
    {
        Ptr pItem = nullptr;

        HRESULT hr = List<Ptr>::getItem(pNode, &pItem);
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

    HRESULT removeItem(Node *pNode, Ptr *ppItem)
    {
        Ptr pItem = nullptr;

        HRESULT hr = List<Ptr>::removeItem(pNode, &pItem);

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
