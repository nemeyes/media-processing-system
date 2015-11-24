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
 * @file <App.xaml.cpp>
 *
 * @brief Implementation of the App class.
 *        Member variables in the App class are in scope throughout the app.
 *        App.xaml has no visual design surface, but we can still use the
 *        document outline and property inspector in the desiger.
 *
 *******************************************************************************
 */

#include "pch.h"
#include "MainPage.xaml.h"
#include "HardwareCheck.xaml.h"

#include "common.h"

using namespace TranscodeVqWinStore;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

/**
 *******************************************************************************
 *  @fn     App()
 *  @brief  Initializes the singleton application object.  This is the first
 *          line of authored code executed, and as such is the logical
 *          equivalent of main() or WinMain().
 *
 *******************************************************************************
 */
App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

/**
 *******************************************************************************
 *  @fn     OnLaunched()
 *  @brief  Invoked when the application is launched normally by the end user.
 *          Other entry points will be used when the application is launched to
 *          open a specific file, to display search results, and so forth.
 *  @param[in] args: Details about the launch request and process
 *
 *******************************************************************************
 */
void App::OnLaunched(Windows::ApplicationModel::Activation::
    LaunchActivatedEventArgs^ args)
{
    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    /**************************************************************************
     * Do not repeat app initialization when the Window already has content,  *
     * just ensure that the window is active                                  *
     **************************************************************************/
    if (rootFrame == nullptr)
    {
        /**********************************************************************
         * Create a Frame to act as the navigation context and associate it   *
         * with a SuspensionManager key                                       *
         **********************************************************************/
        rootFrame = ref new Frame();

        /**********************************************************************
         * Place the frame in the current Window                              *
         **********************************************************************/
        Window::Current->Content = rootFrame;
    }

    if (rootFrame->Content == nullptr)
    {
        if (!rootFrame->Navigate(TypeName(TranscodeVqWinStore::HardwareCheck::
            typeid), MainPage::typeid))
        {
            throw ref new FailureException("Failed to create start page");
        }
    }

    /**************************************************************************
     * Ensure the current window is active                                    *
     **************************************************************************/
    Window::Current->Activate();
}

/**
 *******************************************************************************
 *  @fn     OnSuspending()
 *  @brief  Invoked when application execution is being suspended.  Application
 *          state is saved without knowing whether the application will be
 *          terminated or resumed with the contents of memory still intact.
 *  @param[in] sender: The source of the suspend request
 *  @parma[in] e: Details about the suspend request
 *
 *******************************************************************************
 */
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void) sender; // Unused parameter
    (void) e; // Unused parameter

    //TODO: Save application state and stop any background activity
}
