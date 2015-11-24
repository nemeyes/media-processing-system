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

namespace TranscodeVqWinStore {
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
    /**
     * A virual function
     */
    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::
        NavigationEventArgs^ e) override;

private:

    typedef void (TranscodeVqWinStore::MainPage::
        *EVENT_HANDLER_FUNCTION_POINTER)(void);

    /**
     * @brief pickfile(). Occurs when pickfileButton clicked
     */
    void pickfile(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief transcodeWithVQ(). Occurs when transcodeButton clicked
     */
    void transcodeWithVQ(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief transcodeCancel(). Occurs when transcodeCancelButton clicked
     */
    void transcodeCancel(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    /**
     * @brief transcodeProgress(). Shows the transcoding progress
     */
    void transcodeProgress(Windows::Foundation::
        IAsyncActionWithProgress<double>^ asyncInfo, double percent);
    /**
     * @brief transcodeError(). Shows the transcode error
     */
    void transcodeError(Platform::String^ error);
    /**
     * @brief transcodeFailure(). Shows the transcode failure reasons
     */
    void transcodeFailure(Windows::Media::Transcoding::TranscodeFailureReason
        reason);
    /**
     * @brief setCancelButton(). Enables/Disables the cancel button
     */
    void setCancelButton(bool isEnabled);
    /**
     * @brief enableButtons(). enabling the buttons
     */
    void enableButtons();
    /**
     * @brief disableButtons(). disabling the buttons
     */
    void disableButtons();
    /**
     * @brief playFile(). Playing a media file with AMF VQ effects
     */
    void MainPage::playFile(Windows::Storage::StorageFile^ MediaFile);

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

    /**
     * @brief changeEncoderParameters(). setting up the encode params - bitrate,
     * qualityVsSpeed, gopSize
     */
    void changeEncoderParameters();
    /**
     * @brief updateFromControls(). VQ update from UI controls
     */
    void updateFromControls();

    /**
     * @brief updateScale(). Scaling params update
     */
    void updateScale();

private:
    void updateVqControls(Windows::Foundation::Collections::PropertySet^
        vqPropertySet);
    void setRecommendedSettings(Platform::Object^ sender, Windows::UI::Xaml::
        RoutedEventArgs^ e);

    /**< Output file name */
    Platform::String^ outputFileName;
    /**< Media encoding profile */
    Windows::Media::MediaProperties::MediaEncodingProfile^ profile;
    int deinterlaceMethod;
    Windows::Storage::StorageFile^ inputFile; /**< Input file */
    Windows::Storage::StorageFile^ outputFile; /**< Transcoded output file */
    /**< Media transcoder, used for transcoding video and audio files */
    Windows::Media::Transcoding::MediaTranscoder^ transcoder;
    Concurrency::cancellation_token_source cts; /**< Cancellation token */
    /**< Property set used for VQ effect options */
    Windows::Foundation::Collections::PropertySet^ propertySet;
    /**< Property set used for transcoding */
    Windows::Foundation::Collections::PropertySet^ propertySetTranscode;
    /**< Property set used for previewing */
    Windows::Foundation::Collections::PropertySet^ propertySetPreview;

    bool isVqSupported;
};
}
