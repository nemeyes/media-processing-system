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
 * @file <MainPage.xaml.h>
 *
 * @brief Contains the declaration of the MianPage.xaml class.
 *
 *******************************************************************************
 */

#pragma once

#include "MainPage.g.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Transcoding;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

namespace PlaybackVqWinStore {
/**
 * MainPage
 */
public ref class MainPage sealed
{
public:
    /**
     * A constructor
     */
    MainPage();

protected:
    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::
        NavigationEventArgs^ e) override;

private:

    typedef void (PlaybackVqWinStore::MainPage::*EVENT_HANDLER_FUNCTION_POINTER)
        (void);

    /**
     * @brief pickfile(). Occurs when pickfileButton clicked
     */
    void pickfile(/*Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e*/);
    /**
     * @brief transcodeCancel(). Occurs when transcodeCancelButton clicked
     */
    void playbackCancel(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief playbackError(). Shows the transcode error
     */
    void playbackError(Platform::String^ error);
    /**
     * @brief dispatcherTimerTick()
     */
    void dispatcherTimerTick(Platform::Object^ sender, Platform::Object^ e);
    /**
     * @brief updateFromControls(). Controlling the VQ options
     */
    void updateFromControls();
    /**
     * @brief play(). Playing a media file with AMF VQ effects
     */
    void play();
    /**
     * @brief pause(). Pausing the playback
     */
    void pause();
    /**
     * @brief resume(). Resuming the playback
     */
    void resume();
    /**
     * @brief stop(). Stops the playback
     */
    void stop();
    /**
     * @brief PlayButtonClick(). Occurs when play button clicked
     */
    void playButtonClick(Object^ pSender,
        Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief pauseButtonClick(). Occurs when pause button clicked
     */
    void pauseButtonClick(Object^ pSender,
        Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief stopButtonClick(). Occurs when stop button clicked
     */
    void stopButtonClick(Object^ pSender,
        Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief initDefaults(). Initialization and handling of UI controls
     */
    void initDefaults();
    /**
     * @brief initSlider(). Intializing the slider
     */
    void initSlider( Windows::UI::Xaml::Controls::Slider^ slider,
        EVENT_HANDLER_FUNCTION_POINTER pEventHandler, double minValue,
        double maxValue, double defaultValue, double step = 1.0);
    /**
     * @brief initCheckBox(). Initializing the checkbox
     */
    void initCheckBox( Windows::UI::Xaml::Controls::CheckBox^ checkBox,
        EVENT_HANDLER_FUNCTION_POINTER pEventHandler, bool isChecked);
    /**
     * @brief initRadioButton(). Intializing the radio button
     */
    void initRadioButton( Windows::UI::Xaml::Controls::RadioButton^ radioButton,
        EVENT_HANDLER_FUNCTION_POINTER pEventHandler, bool isChecked);

private:
    void updateVqControls(Windows::Foundation::Collections::PropertySet^
        vqPropertySet);
    void setRecommendedSettings(void);

    Windows::Foundation::EventRegistrationToken pauseResumeEventRegToken;
    /**< Input file */
    Windows::Storage::StorageFile^ inputFile;
    /**< Cancellation token */
    Concurrency::cancellation_token_source cts;
    /**< Property set used for VQ effect options */
    Windows::Foundation::Collections::PropertySet^ propertySet;
    bool isPlaying;
    bool isVqSupported;
    int deinterlaceMethod;
};
}
