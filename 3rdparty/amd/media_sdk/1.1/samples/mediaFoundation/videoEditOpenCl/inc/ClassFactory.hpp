/**
 * @file <ClassFactory.hpp>
 */

#pragma once

typedef HRESULT (*FNCREATEINSTANCE)(REFIID riid, void **ppvObject);
typedef void (*FNDLLADDREF)();
typedef void (*FNDLLRELEASE)();

/**
 *   @class ClassFactory.
 */
class ClassFactory: public IClassFactory
{
public:

    /**
     *   @brief Fabric method.
     */
    template<const CLSID* MFT_CLSID>
    static HRESULT CreateInstance(FNCREATEINSTANCE createMft,
                    FNDLLADDREF fnAddRef, FNDLLRELEASE fnRelease,
                    REFCLSID clsid, REFIID riid, void **ppv)
    {
        if (nullptr == ppv)
        {
            return E_POINTER;
        }

        if (clsid != *MFT_CLSID)
        {
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        if (nullptr == createMft || nullptr == fnAddRef || nullptr == fnRelease)
        {
            return E_INVALIDARG;
        }

        *ppv = NULL;

        CComPtr < IClassFactory > classFactory
                        = new (std::nothrow) ClassFactory(createMft, fnAddRef,
                                        fnRelease);
        if (nullptr == classFactory)
        {
            return E_OUTOFMEMORY;
        }

        return classFactory->QueryInterface(riid, ppv);
    }

    /**
     *   @brief IUnknown::QueryInterface().
     */
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] = { QITABENT(ClassFactory, IClassFactory),
                                      { 0 } };

        return QISearch(this, qit, riid, ppv);
    }

    /**
     *   @brief IUnknown::AddRef().
     */
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_referenceCounter);
    }

    /**
     *   @brief IUnknown::Release().
     */
    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_referenceCounter);
        if (cRef == 0)
        {
            delete this;
        }

        return cRef;
    }

    /**
     *   @brief IClassFactory::CreateInstance().
     */
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        return punkOuter ? CLASS_E_NOAGGREGATION : _createInstance(riid, ppv);
    }

    /**
     *   @brief IClassFactory::LockServer().
     */
    IFACEMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
        {
            _dllAddRef();
        }
        else
        {
            _dllRelease();
        }

        return S_OK;
    }

private:

    ClassFactory(FNCREATEINSTANCE createInstance, FNDLLADDREF dllAddRef,
                    FNDLLRELEASE dllRelease) :
        _referenceCounter(0), _createInstance(createInstance), _dllAddRef(
                        dllAddRef), _dllRelease(dllRelease)
    {
        _dllAddRef();
    }

    ~ClassFactory()
    {
        _dllRelease();
    }

    long _referenceCounter;
    FNCREATEINSTANCE _createInstance;
    FNDLLADDREF _dllAddRef;
    FNDLLRELEASE _dllRelease;
};
