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
 ******************************************************************************/

/**
 *******************************************************************************
 * @file <common.cpp>
 *
 * @brief Source file for VqHelpers functionality1
 *
 *******************************************************************************
 */
#include "pch.h"

#include <d3d11.h>
#include <mfapi.h>
#include <wrl\wrappers\corewrappers.h>

#include "common.h"

using Platform::FailureException;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Wrappers::HStringReference;
using Windows::Foundation::ActivateInstance;
using Windows::Foundation::Collections::PropertySet;

namespace VqHelpers
{
    /**
     * CacheBuilderSubscriber
     */
    class CacheBuilderSubscriber : public IAMFCacheBuilderEvents
    {
    public:
        /**
         * Constructor
         */
        CacheBuilderSubscriber(std::function<void(unsigned)> callback)
            : _progress(),
            _refCount(0),
            _callback(callback)
        { }

        /**
         * Invoke()
         */
        virtual HRESULT STDMETHODCALLTYPE Invoke(UINT nPercent)
        {
            if (_progress < nPercent)
            {
                _progress = nPercent;

                if (_callback != nullptr)
                {
                    _callback(nPercent);
                }
            }

            return S_OK;
        }

        /**
         * QueryInterface()
         */
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
             void** ppvObject)
        {
            if (nullptr == ppvObject)
            {
                return E_POINTER;
            }

            if (riid == __uuidof(IAMFCapabilityManager))
            {
                *ppvObject = static_cast<IAMFCacheBuilderEvents*>(this);

                AddRef();

                return S_OK;
            }
            else if (riid == __uuidof(IUnknown))
            {
                *ppvObject = static_cast<IUnknown*>(this);

                AddRef();

                return S_OK;
            }

            return E_NOINTERFACE;
        }

        /**
         * AddRef()
         */
        virtual ULONG STDMETHODCALLTYPE AddRef(void)
        {
            return InterlockedIncrement(&_refCount);
        }

        /**
         * Release()
         */
        virtual ULONG STDMETHODCALLTYPE Release(void)
        {
            unsigned long refCount = InterlockedDecrement(&_refCount);
            if (0 == refCount)
            {
                delete this;
            }

            return refCount;
        }

        /**
         * GetIids()
         */
        virtual HRESULT STDMETHODCALLTYPE GetIids(ULONG *iidCount, IID **iids)
        {
            return E_NOTIMPL;
        }

        /**
         * GetRuntimeClassName()
         */
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeClassName(
            HSTRING *className)
        {
            return E_NOTIMPL;
        }

        /**
         * GetTrustLevel()
         */
        virtual HRESULT STDMETHODCALLTYPE GetTrustLevel(TrustLevel *trustLevel)
        {
            return E_NOTIMPL;
        }

    private:
        unsigned _progress;
        __declspec(align(4)) ULONG _refCount;
        std::function<void(unsigned)> _callback;
    };

    /**
     * ActivateAmfCacheBuilder()
     */
    std::pair<HRESULT, ComPtr<IAMFCacheBuilder>> ActivateAmfCacheBuilder()
    {
        ComPtr<IAMFCacheBuilder> instance;
        HRESULT hr = ActivateInstance(HStringReference(
            RuntimeClass_mftvqLib_AMFCacheBuilder).Get(), &instance);

        return std::make_pair(hr, instance);
    }

    /**
     * ActivateAmfCapabilityManager()
     */
    std::pair<HRESULT, ComPtr<IAMFCapabilityManager>>
        ActivateAmfCapabilityManager()
    {
        ComPtr<IAMFCapabilityManager> instance;
        HRESULT hr = ActivateInstance(HStringReference(
            RuntimeClass_mftvqLib_AMFCapabilityManager).Get(), &instance);

        return std::make_pair(hr, instance);
    }

    /**
     * BuildVqCache()
     */
    concurrency::task<HRESULT> BuildVqCache(std::function<void(
        unsigned progressPercent)> progressCallback)
    {
        if (nullptr == progressCallback)
        {
            throw ref new FailureException("Invalid argument.");
        }

        return concurrency::create_task([progressCallback]() -> HRESULT
        {
            ComPtr<IAMFCacheBuilder> cacheBuilder;
            HRESULT hr = ActivateInstance(HStringReference(
                RuntimeClass_mftvqLib_AMFCacheBuilder).Get(), &cacheBuilder);
            if (FAILED(hr))
            {
                return hr;
            }

            ComPtr<IAMFCacheBuilderEvents> cacheBuildSubscriber(new VqHelpers::
                CacheBuilderSubscriber(progressCallback));

            return cacheBuilder->BuildCache(cacheBuildSubscriber.Get());
        });
    }

    /**
     * GetRecommendedSettings()
     */
    PropertySet^ GetRecommendedSettings(
        DWORD width,
        DWORD height,
        BOOL interlaceMode,
        DWORD deinterlaceMethod,
        AMFCMRequestType requestType)
    {
        ComPtr<IAMFCapabilityManager> capabilityManager;
        HRESULT hr = ActivateInstance(HStringReference(
            RuntimeClass_mftvqLib_AMFCapabilityManager).Get(),
            &capabilityManager);
        if (FAILED(hr))
        {
            throw ref new FailureException("Failed to create a VQ capability   \
                manager.");
        }

        hr = capabilityManager->Init(width, height, interlaceMode,
            deinterlaceMethod);
        if (hr != S_OK && hr != S_FALSE)
        {
            throw ref new FailureException("IAMFCapabilityManager::Init()      \
                failed. Platform does not support VQ filters.");
        }

        auto isEnabled = [](ComPtr<IAMFCapabilityManager>&capabilityManager,
            AMFCMRequestType requestType, const wchar_t*
            featureName) -> bool
        {
            HRESULT hr = capabilityManager->IsEnabled(requestType,
                HStringReference(featureName).Get());
            if (FAILED(hr))
            {
                throw ref new FailureException("IAMFCapabilityManager::        \
                    IsEnabled() failed.");
            }

            return S_OK == hr;
        };

        PropertySet^ propertySet(ref new PropertySet);

        auto setIfEnabled = [&isEnabled, &propertySet, &capabilityManager,
            requestType](const wchar_t* featureName) -> void
        {
            bool enabled = isEnabled(capabilityManager, requestType,
                featureName);

            propertySet->Insert(ref new Platform::String(featureName), enabled);
        };

        /**********************************************************************
         * Note: AMF_EFFECT_DYNAMIC_RANGE and AMF_EFFECT_DEINTERLACING are    *
         * always available                                                   *
         **********************************************************************/

        setIfEnabled(AMF_EFFECT_STEADY_VIDEO);
        setIfEnabled(AMF_EFFECT_EDGE_ENHANCEMENT);
        setIfEnabled(AMF_EFFECT_DENOISE);
        setIfEnabled(AMF_EFFECT_FALSE_CONTOUR_REDUCTION);
        setIfEnabled(AMF_EFFECT_MOSQUITO_NOISE);
        setIfEnabled(AMF_EFFECT_DEBLOCKING);
        setIfEnabled(AMF_EFFECT_DYNAMIC_CONTRAST);
        setIfEnabled(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION);
        setIfEnabled(AMF_EFFECT_COLOR_VIBRANCE);
        setIfEnabled(AMF_EFFECT_SKINTONE_CORRECTION);
        setIfEnabled(AMF_EFFECT_BRIGHTER_WHITES);
        setIfEnabled(AMF_EFFECT_GAMMA_CORRECTION);
        setIfEnabled(AMF_EFFECT_DEMOMODE);

        return propertySet;
    }
}
