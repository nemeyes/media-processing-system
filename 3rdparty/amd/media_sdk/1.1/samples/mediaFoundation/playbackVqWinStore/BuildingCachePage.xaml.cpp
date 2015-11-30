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
 * @file <BuildingCachePage.xaml.cpp>
 *
 * @brief This containes BuildingCachePage
 *
 *******************************************************************************
 */

#include "pch.h"
#include <vccorlib.h>
#include "BuildingCachePage.xaml.h"

#include "common.h"

using namespace PlaybackVqWinStore;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

/**
 *******************************************************************************
 *  @fn     BuildingCachePage()
 *  @brief  Constructor of the class BuildingCachePage
 *          The Blank Page item template is documented at
 *          http://go.microsoft.com/fwlink/?LinkId=234238
 *
 *******************************************************************************
 */
BuildingCachePage::BuildingCachePage()
{
    InitializeComponent();

    Windows::UI::Core::CoreDispatcher^ dispatcher = Window::Current->Dispatcher;

    VqHelpers::BuildVqCache([this, dispatcher](unsigned progressPercent)
    {
        dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler(
            [this, progressPercent]()
        {
                buildProgressBar->Value = progressPercent;
        }) );
    }).then([this, dispatcher](concurrency::task<HRESULT> result)
    {
        bool enableVQ = result.get() == S_OK;

        dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler(
            [this, enableVQ]()
        {
            Windows::UI::Xaml::Interop::TypeName returnPageTypeName = Windows::
                UI::Xaml::Interop::TypeName(returnPageType);

            if (!this->Frame->Navigate(returnPageTypeName, Platform::
                Boolean(enableVQ)))
            {
                throw ref new FailureException("Failed to load main page");
            }
        }) );
    });
}

/**
 *******************************************************************************
 *  @fn     OnNavigatedTo()
 *  @brief  Invoked when this page is about to be displayed in a Frame.
 *
 *  @param[in] e : Event data that describes how this page was reached. The
 *                 Parameter property is typically used to configure the page.
 *
 *******************************************************************************
 */
void BuildingCachePage::OnNavigatedTo(NavigationEventArgs^ e)
{
    returnPageType = dynamic_cast<Platform::Type^>(e->Parameter);
}
