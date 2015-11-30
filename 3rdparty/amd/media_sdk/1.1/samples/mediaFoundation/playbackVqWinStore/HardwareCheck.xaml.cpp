/******************************************************************************
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
 * @file <HardwareCheck.xaml.cpp>
 *
 * @brief Implementation of the HardwareCheck class
 *
 *******************************************************************************
 */

#include "pch.h"

#include <wrl\wrappers\corewrappers.h>

#include "HardwareCheck.xaml.h"
#include "BuildingCachePage.xaml.h"

#include "common.h"

using namespace PlaybackVqWinStore;
using namespace Windows::UI::Xaml;

using Microsoft::WRL::ComPtr;
using Windows::UI::Xaml::Interop::TypeName;
using Windows::UI::Core::DispatchedHandler;
using Windows::UI::Core::CoreDispatcherPriority;

HardwareCheck::HardwareCheck()
{
    InitializeComponent();

    Windows::UI::Core::CoreDispatcher^ dispatcher = Window::Current->Dispatcher;

    concurrency::create_async([this, dispatcher]()
    {
        auto goToApplicationPage = [this, dispatcher](bool enableVq)
        {
            dispatcher->RunAsync(CoreDispatcherPriority::Normal,
                ref new DispatchedHandler([this, enableVq]()
            {
                if (!this->Frame->Navigate(TypeName(returnPageType), Platform::Boolean(enableVq)))
                {
                    throw ref new Platform::FailureException("Failed to load main page");
                }
            }));
        };

        auto goToBuildPage = [this, dispatcher]()
        {
            dispatcher->RunAsync(CoreDispatcherPriority::Normal,
                ref new DispatchedHandler([this]()
            {
                if (!this->Frame->Navigate(TypeName(BuildingCachePage::typeid), this->returnPageType))
                {
                    throw ref new Platform::FailureException("Failed to load main page");
                }
            }));
        };

        auto capabilityManagerActivationResult = VqHelpers::ActivateAmfCapabilityManager();
        if (capabilityManagerActivationResult.first != S_OK)
        {
            goToApplicationPage(false);
        }
        else if (capabilityManagerActivationResult.second->Init(1920, 1080) != S_OK)
        {
            goToApplicationPage(false);
        }
        else
        {
            auto cacheBuilderActivationResult = VqHelpers::ActivateAmfCacheBuilder();
            if (cacheBuilderActivationResult.first != S_OK)
            {
                goToApplicationPage(false);
            }
            else
            {
                HRESULT hr = cacheBuilderActivationResult.second->IsBuildCacheRequired();

                if (S_OK == hr)
                {
                    goToBuildPage();
                }
                else if (S_FALSE == hr)
                {
                    goToApplicationPage(true);
                }
                else
                {
                    goToApplicationPage(false);
                }
            }
        }
    });
}

void HardwareCheck::OnNavigatedTo(Navigation::NavigationEventArgs^ e)
{
    returnPageType = dynamic_cast<Platform::Type^>(e->Parameter);
}
