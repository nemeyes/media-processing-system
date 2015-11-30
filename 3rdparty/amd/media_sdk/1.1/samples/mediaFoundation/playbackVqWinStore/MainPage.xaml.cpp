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
 * @file <MainPage.xaml.cpp>
 *
 * @brief The implementation code-behind files for the Transcode Application and
 *        MianPage classes. In this file we can add event handlers and other
 *        custom program logic that's related to this page. variables in the
 *        Page are in scope only in that page.
 *
 *******************************************************************************
 */
#include "pch.h"
#include "MainPage.xaml.h"
#include <Codecapi.h>

#include "common.h"

using namespace PlaybackVqWinStore;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Storage::Streams;
using namespace concurrency;

/**
 *******************************************************************************
 *  @fn     MainPage()
 *  @brief  Constructor of the class MainPage
 *
 *******************************************************************************
 */
MainPage::MainPage()
    : propertySet(ref new PropertySet()),
    isPlaying(false)
{
    InitializeComponent();

    initDefaults();

    statusMessage->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    TimeSpan t;
    t.Duration = 1000;

    DispatcherTimer^ timer = ref new DispatcherTimer;
    timer->Tick += ref new EventHandler<Object^>(this,
        &MainPage::dispatcherTimerTick);
    timer->Interval = t;
    timer->Start();
}

/**
 *******************************************************************************
 *  @fn     dispatcherTimerTick
 *  @brief
 *
 *  @param[in] sender:
 *  @param[in] e     :
 *
 *  @return
 *******************************************************************************
 */
void MainPage::dispatcherTimerTick(Platform::Object^ sender,
    Platform::Object^ e)
{
    if(propertySet && propertySet->HasKey(AMF_PROCESSING_TRANSFORM_AVR_TIME))
    {
        String^ stansformTime = propertySet->Lookup(
            AMF_PROCESSING_TRANSFORM_AVR_TIME)->ToString();
        transcodeProcessTimeText->Text = "VQ transform time: " + stansformTime +
            " ms";
    }
    else
    {
        transcodeProcessTimeText->Text = "VQ transform time: -- ms";
    }
}

/**
 *******************************************************************************
 *  @fn     playbackError
 *  @brief  Shows the transcode error
 *
 *  @param[in] Error: Transcoding error
 *
 *  @return
 *******************************************************************************
 */
void MainPage::playbackError(Platform::String^ error)
{
    /**************************************************************************
     * Updating transcoding errors                                            *
     **************************************************************************/
    statusMessage->Visibility = Windows::UI::Xaml::Visibility::Visible;
    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
        SolidColorBrush(Windows::UI::Colors::Red);
    statusMessage->Text = error;
}

/**
 *******************************************************************************
 *  @fn     initDefaults
 *  @brief  Initialization and handling of UI controls
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::initDefaults()
{
    initSlider(zoomSlider, &MainPage::updateFromControls, 90, 100, 100);
    initSlider(delaySlider, &MainPage::updateFromControls, 0, 6, 1);
    initSlider(strengthSlider, &MainPage::updateFromControls, 0, 3, 3);
    initSlider(edgeEnhancementSlider, &MainPage::updateFromControls, 1, 100,
        50);
    initSlider(denoiseSlider, &MainPage::updateFromControls, 1, 100, 50);
    initSlider(mosquitoSlider, &MainPage::updateFromControls, 1, 100, 68);
    initSlider(deblockingSlider, &MainPage::updateFromControls, 0, 100, 50);
    initSlider(brightnessSlider, &MainPage::updateFromControls, -100, 100, 0);
    initSlider(contrastSlider, &MainPage::updateFromControls, 0, 2, 1, 0.01);
    initSlider(saturationSlider, &MainPage::updateFromControls, 0, 2, 1, 0.01);
    initSlider(tintSlider, &MainPage::updateFromControls, -30.0, 30.0, 0.0,
        0.1);
    initSlider(colorVibranceSlider, &MainPage::updateFromControls, 1, 100, 40);
    initSlider(fleshToneCorrectionSlider, &MainPage::updateFromControls, 1, 100,
        50);
    initSlider(videoGammaSlider, &MainPage::updateFromControls, 0.5, 2.5, 1.0,
        0.01);
    initSlider(falseContourReduceSlider,&MainPage::updateFromControls, 0, 100,
        50, 1);

    initRadioButton(fullRangeRadioButton, &MainPage::updateFromControls, true);
    initRadioButton(limitedRangeRadioButton, &MainPage::updateFromControls,
        false);
    initRadioButton(autoRadioButton, &MainPage::updateFromControls, true);
    initRadioButton(weaveRadioButton, &MainPage::updateFromControls, false);
    initRadioButton(bobRadioButton, &MainPage::updateFromControls, false);
    initRadioButton(adaptiveRadioButton, &MainPage::updateFromControls, false);
    initRadioButton(motionAdaptiveRadioButton, &MainPage::updateFromControls,
        false);
    initRadioButton(vectorAdaptiveRadioButton, &MainPage::updateFromControls,
        false);

    initCheckBox(dynamicContrastCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(pulldownDetectionCheckBox, &MainPage::updateFromControls,
        false);
    initCheckBox(steadyVideoCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(brighterWhitesCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(dynamicRangeCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(videoGammaCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(colorVibranceCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(fleshToneCorrectionCheckBox, &MainPage::updateFromControls,
        false);
    initCheckBox(edgeEnhancementCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(deblockingCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(mosquitoCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(denoiseCheckBox, &MainPage::updateFromControls, false);
    initCheckBox(falseContourReduceCheckBox, &MainPage::updateFromControls,
        false);
    initCheckBox(demoModeCheckBox, &MainPage::updateFromControls, false);

    updateFromControls();
}

/**
 *******************************************************************************
 *  @fn     updateFromControls
 *  @brief  Updating Property set from UI controls
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::updateFromControls()
{
    propertySet->Insert(AMF_EFFECT_STEADY_VIDEO,
        steadyVideoCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_STEADY_VIDEO_ZOOM, zoomSlider->Value);
    propertySet->Insert(AMF_EFFECT_STEADY_VIDEO_DELAY, delaySlider->Value);
    propertySet->Insert(AMF_EFFECT_STEADY_VIDEO_STRENGTH,
        strengthSlider->Value);

    propertySet->Insert(AMF_EFFECT_EDGE_ENHANCEMENT,
        edgeEnhancementCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH,
        edgeEnhancementSlider->Value);

    propertySet->Insert(AMF_EFFECT_DENOISE, denoiseCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_DENOISE_STRENGTH, denoiseSlider->Value);

    propertySet->Insert(AMF_EFFECT_FALSE_CONTOUR_REDUCTION,
        falseContourReduceCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH,
        falseContourReduceSlider->Value);

    propertySet->Insert(AMF_EFFECT_MOSQUITO_NOISE,
        mosquitoCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_MOSQUITO_NOISE_STRENGTH,
        mosquitoSlider->Value);

    propertySet->Insert(AMF_EFFECT_DEBLOCKING,
        deblockingCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_DEBLOCKING_STRENGTH,
        deblockingSlider->Value);

    propertySet->Insert(AMF_EFFECT_GAMMA_CORRECTION,
        videoGammaCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_GAMMA_CORRECTION_STRENGTH,
        videoGammaSlider->Value);

    propertySet->Insert(AMF_EFFECT_BRIGHTNESS, brightnessSlider->Value);
    propertySet->Insert(AMF_EFFECT_CONTRAST, contrastSlider->Value);
    propertySet->Insert(AMF_EFFECT_SATURATION, saturationSlider->Value);
    propertySet->Insert(AMF_EFFECT_TINT, tintSlider->Value);

    propertySet->Insert(AMF_EFFECT_COLOR_VIBRANCE,
        colorVibranceCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_COLOR_VIBRANCE_STRENGTH,
        colorVibranceSlider->Value);

    propertySet->Insert(AMF_EFFECT_SKINTONE_CORRECTION,
        fleshToneCorrectionCheckBox->IsChecked->Value);
    propertySet->Insert(AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH,
        fleshToneCorrectionSlider->Value);

    propertySet->Insert(AMF_EFFECT_BRIGHTER_WHITES,
        brighterWhitesCheckBox->IsChecked->Value);

    propertySet->Insert(AMF_EFFECT_DYNAMIC_CONTRAST,
        dynamicContrastCheckBox->IsChecked->Value);

    propertySet->Insert(AMF_EFFECT_DEMOMODE,
        demoModeCheckBox->IsChecked->Value);

    if (dynamicRangeCheckBox->IsChecked->Value)
    {
        int dynamicRangeValue = fullRangeRadioButton->IsChecked->Value ?
            AMF_EFFECT_DYNAMIC_RANGE_FULL : AMF_EFFECT_DYNAMIC_RANGE_LIMITED;

        propertySet->Insert(AMF_EFFECT_DYNAMIC_RANGE, dynamicRangeValue);
    }
    else
    {
        propertySet->Insert(AMF_EFFECT_DYNAMIC_RANGE, ref new Platform::
            Box<uint32_t>(AMF_EFFECT_DYNAMIC_RANGE_NONE));
    }

    propertySet->Insert(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION,
        pulldownDetectionCheckBox->IsChecked->Value);

    deinterlaceMethod = AMF_EFFECT_DEINTERLACING_AUTOMATIC;
    if (weaveRadioButton->IsChecked->Value)
    {
        deinterlaceMethod = AMF_EFFECT_DEINTERLACING_WEAVE;
    }
    else if (bobRadioButton->IsChecked->Value)
    {
        deinterlaceMethod = AMF_EFFECT_DEINTERLACING_BOBE;
    }
    else if (adaptiveRadioButton->IsChecked->Value)
    {
        deinterlaceMethod = AMF_EFFECT_DEINTERLACING_ADAPTIVE;
    }
    else if (motionAdaptiveRadioButton->IsChecked->Value)
    {
        deinterlaceMethod = AMF_EFFECT_DEINTERLACING_MOTION_ADAPTIVE;
    }
    else if (vectorAdaptiveRadioButton->IsChecked->Value)
    {
        deinterlaceMethod = AMF_EFFECT_DEINTERLACING_VECTOR_ADAPTIVE;
    }
    propertySet->Insert(AMF_EFFECT_DEINTERLACING, deinterlaceMethod);

    updateVqControls(propertySet);
}

/**
 *******************************************************************************
 *  @fn     updateVqControls
 *  @brief  Updating VQ params from property set
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::updateVqControls(Windows::Foundation::Collections::PropertySet^
    vqPropertySet)
{
    if(isVqSupported == true)
    {
        auto isEnabled = [&vqPropertySet, this](const wchar_t* featureName)
        {
            Platform::String^ name = ref new Platform::String(featureName);

            bool hasKey = vqPropertySet->HasKey(name);

            Windows::Foundation::IPropertyValue^ val = dynamic_cast<Windows::
                Foundation::IPropertyValue^>(vqPropertySet->Lookup(name));

            bool enabled = val->GetBoolean() && this->isVqSupported;

            return hasKey && enabled;
        };

        auto updateSlider = [&vqPropertySet, this](
            bool enabled,
            const wchar_t* parameterName,
            Windows::UI::Xaml::Controls::Slider^ slider)
        {
            slider->IsEnabled = enabled && this->isVqSupported;

            Platform::String^ name = ref new Platform::String(parameterName);
            if (vqPropertySet->HasKey(name))
            {
                Windows::Foundation::IPropertyValue^ value =
                    dynamic_cast<Windows::Foundation::
                    IPropertyValue^>(vqPropertySet->Lookup(name));
                slider->Value = value->GetDouble();
            }
        };

        steadyVideoCheckBox->IsChecked = isEnabled(AMF_EFFECT_STEADY_VIDEO);
        updateSlider(steadyVideoCheckBox->IsChecked->Value,
            AMF_EFFECT_STEADY_VIDEO_ZOOM, zoomSlider);
        updateSlider(steadyVideoCheckBox->IsChecked->Value,
            AMF_EFFECT_STEADY_VIDEO_DELAY, delaySlider);
        updateSlider(steadyVideoCheckBox->IsChecked->Value,
            AMF_EFFECT_STEADY_VIDEO_STRENGTH, strengthSlider);

        edgeEnhancementCheckBox->IsChecked =
            isEnabled(AMF_EFFECT_EDGE_ENHANCEMENT);
        updateSlider(edgeEnhancementCheckBox->IsChecked->Value,
            AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH, edgeEnhancementSlider);

        denoiseCheckBox->IsChecked = isEnabled(AMF_EFFECT_DENOISE);
        updateSlider(denoiseCheckBox->IsChecked->Value,
            AMF_EFFECT_DENOISE_STRENGTH, denoiseSlider);

        mosquitoCheckBox->IsChecked = isEnabled(AMF_EFFECT_MOSQUITO_NOISE);
        updateSlider(mosquitoCheckBox->IsChecked->Value,
            AMF_EFFECT_MOSQUITO_NOISE_STRENGTH, mosquitoSlider);

        deblockingCheckBox->IsChecked = isEnabled(AMF_EFFECT_DEBLOCKING);
        updateSlider(deblockingCheckBox->IsChecked->Value,
            AMF_EFFECT_DEBLOCKING_STRENGTH, deblockingSlider);

        videoGammaCheckBox->IsChecked = isEnabled(AMF_EFFECT_GAMMA_CORRECTION);
        updateSlider(videoGammaCheckBox->IsChecked->Value,
            AMF_EFFECT_GAMMA_CORRECTION_STRENGTH, videoGammaSlider);

        fleshToneCorrectionCheckBox->IsChecked = isEnabled(
            AMF_EFFECT_SKINTONE_CORRECTION);
        updateSlider(fleshToneCorrectionCheckBox->IsChecked->Value,
            AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH, fleshToneCorrectionSlider);

        falseContourReduceCheckBox->IsChecked = isEnabled(
            AMF_EFFECT_FALSE_CONTOUR_REDUCTION);
        updateSlider(falseContourReduceCheckBox->IsChecked->Value,
            AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH,
            falseContourReduceSlider);

        colorVibranceCheckBox->IsChecked = isEnabled(AMF_EFFECT_COLOR_VIBRANCE);
        updateSlider(colorVibranceCheckBox->IsChecked->Value,
            AMF_EFFECT_COLOR_VIBRANCE_STRENGTH, colorVibranceSlider);

        updateSlider(true, AMF_EFFECT_BRIGHTNESS, brightnessSlider);
        updateSlider(true, AMF_EFFECT_CONTRAST, contrastSlider);
        updateSlider(true, AMF_EFFECT_SATURATION, saturationSlider);
        updateSlider(true, AMF_EFFECT_TINT, tintSlider);

        dynamicContrastCheckBox->IsChecked =
            isEnabled(AMF_EFFECT_DYNAMIC_CONTRAST);

        brighterWhitesCheckBox->IsChecked =
            isEnabled(AMF_EFFECT_BRIGHTER_WHITES);

        demoModeCheckBox->IsChecked = isEnabled(AMF_EFFECT_DEMOMODE);

        if (vqPropertySet->HasKey(AMF_EFFECT_DYNAMIC_RANGE))
        {
            Windows::Foundation::IPropertyValue^ dynamicRangeValue =
                dynamic_cast<Windows::Foundation::IPropertyValue^>(
                    vqPropertySet->Lookup(AMF_EFFECT_DYNAMIC_RANGE));

            long long dynamicRangeInt = dynamicRangeValue->GetInt64();

            fullRangeRadioButton->IsEnabled = dynamicRangeInt !=
                AMF_EFFECT_DYNAMIC_RANGE_NONE;
            limitedRangeRadioButton->IsEnabled = dynamicRangeInt !=
                AMF_EFFECT_DYNAMIC_RANGE_NONE;

            switch (dynamicRangeInt)
            {
            case AMF_EFFECT_DYNAMIC_RANGE_FULL:
                fullRangeRadioButton->IsChecked = true;
                limitedRangeRadioButton->IsChecked = false;
                break;
            case AMF_EFFECT_DYNAMIC_RANGE_LIMITED:
                fullRangeRadioButton->IsChecked = false;
                limitedRangeRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DYNAMIC_RANGE_NONE:
            default:
                fullRangeRadioButton->IsChecked = false;
                limitedRangeRadioButton->IsChecked = false;
                break;
            };
        }

        if (vqPropertySet->HasKey(AMF_EFFECT_DEINTERLACING))
        {
            Windows::Foundation::IPropertyValue^ deinterlacingValue =
                dynamic_cast<Windows::Foundation::IPropertyValue^>(
                vqPropertySet->Lookup(AMF_EFFECT_DEINTERLACING));

            deinterlaceMethod = int(deinterlacingValue->GetInt64());
            switch (deinterlaceMethod)
            {
            case AMF_EFFECT_DEINTERLACING_AUTOMATIC:
                autoRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DEINTERLACING_WEAVE:
                weaveRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DEINTERLACING_BOBE:
                bobRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DEINTERLACING_ADAPTIVE:
                adaptiveRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DEINTERLACING_MOTION_ADAPTIVE:
                motionAdaptiveRadioButton->IsChecked = true;
                break;
            case AMF_EFFECT_DEINTERLACING_VECTOR_ADAPTIVE:
                vectorAdaptiveRadioButton->IsChecked = true;
            default:
                break;
            };
        }
    }
}

/**
 *******************************************************************************
 *  @fn     initSlider
 *  @brief  Initialization of slider
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::initSlider( Windows::UI::Xaml::Controls::Slider^ slider,
    EVENT_HANDLER_FUNCTION_POINTER pEventHandler, double minValue,
    double maxValue, double defaultValue, double step )
{
    slider->StepFrequency = step;
    slider->Minimum = minValue;
    slider->Maximum = maxValue;
    slider->Value = defaultValue;
    slider->ValueChanged += ref new ::Windows::UI::Xaml::Controls::Primitives::
        RangeBaseValueChangedEventHandler(this, (void (MainPage::*)(Platform::
        Object^, Windows::UI::Xaml::Controls::Primitives::
        RangeBaseValueChangedEventArgs^))pEventHandler);
}

/**
 *******************************************************************************
 *  @fn     initCheckBox
 *  @brief  Initialization of check box
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::initCheckBox( Windows::UI::Xaml::Controls::CheckBox^ checkBox,
    EVENT_HANDLER_FUNCTION_POINTER pEventHandler, bool isChecked )
{
    checkBox->IsChecked = isChecked;
    checkBox->Checked += ref new ::Windows::UI::Xaml::RoutedEventHandler(this,
        (void (MainPage::*)(Platform::Object^, Windows::UI::Xaml::
        RoutedEventArgs^))pEventHandler);
    checkBox->Unchecked += ref new ::Windows::UI::Xaml::RoutedEventHandler(this,
        (void (MainPage::*)(Platform::Object^, Windows::UI::Xaml::
        RoutedEventArgs^))pEventHandler);
}

/**
 *******************************************************************************
 *  @fn     initRadioButton
 *  @brief  Initialization of radio button
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::initRadioButton( Windows::UI::Xaml::Controls::RadioButton^
    radioButton, EVENT_HANDLER_FUNCTION_POINTER pEventHandler, bool isChecked )
{
    radioButton->IsChecked = isChecked;
    radioButton->Checked += ref new ::Windows::UI::Xaml::RoutedEventHandler(
        this, (void (MainPage::*)(Platform::Object^, Windows::UI::Xaml::
        RoutedEventArgs^))pEventHandler);
}

/**
 *******************************************************************************
 *  @fn     pickfile
 *  @brief  Occurs when pickfileButton clicked
 *
 *  @param[in] sender:
 *  @param[in] e: state infromation and eavent data of routed event
 *
 *  @return
 *******************************************************************************
 */
void MainPage::pickfile()
{
    try
    {
        ApplicationViewState currentState = ApplicationView::Value;
        if (currentState == ApplicationViewState::Snapped && !ApplicationView::
            TryUnsnap())
        {
            playbackError("Cannot pick files while application is in snapped   \
                view");
        }
        else
        {
            FileOpenPicker^ picker = ref new FileOpenPicker();
            picker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
            picker->FileTypeFilter->Append(".asf");
            picker->FileTypeFilter->Append(".wmv");
            picker->FileTypeFilter->Append(".mpg");
            picker->FileTypeFilter->Append(".mpeg");
            picker->FileTypeFilter->Append(".mp4");
            picker->FileTypeFilter->Append(".avi");

            create_task(picker->PickSingleFileAsync()).then(
            [this](StorageFile^ srcFile)
            {
                inputFile = srcFile;
                if (inputFile != nullptr)
                {
                    play();
                }
            }
            );
        }
    }
    catch(Exception^ exception)
    {
        playbackError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     play
 *  @brief  Playing a media file with AMF VQ effects
 *
 *  @return
 *******************************************************************************
 */
void MainPage::play()
{
    try
    {
        stop();

        //statusMessage->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

        if (inputFile == nullptr)
        {
            pickfile();
        }
        else
        {
            create_task( inputFile->OpenAsync(FileAccessMode::Read)).then(
            [this](IRandomAccessStream^ inputStream)
            {
                try
                {
                    return video->SetSource(inputStream, L"video/mp4");
                }
                catch (Platform::Exception^ exception)
                {
                    playbackError(exception->Message);
                }
                cancel_current_task();
            }).then(
            [this]()
            {
                isPlaying = true;

                video->RemoveAllEffects();
                updateFromControls();
                if(isVqSupported)
                {
                    video->AddVideoEffect("mftvqLib.AMFVideoTransform", true,
                        propertySet);
                }

                statusMessage->Visibility = Windows::UI::Xaml::Visibility::
                    Visible;
                statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
                    SolidColorBrush(Windows::UI::Colors::Green);
                statusMessage->Text = "Playing...";
            });
        }
    }
    catch (Platform::Exception^ exception)
    {
        playbackError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     pause
 *  @brief  pause the playback of the media file
 *
 *  @return
 *******************************************************************************
 */
void MainPage::pause()
{
    try
    {
        if (isPlaying)
        {
            video->Pause();

            pauseResumeButton->Content = L"Resume";
            pauseResumeButton->Click -= pauseResumeEventRegToken;
            pauseResumeEventRegToken = pauseResumeButton->Click += ref new
                RoutedEventHandler(this, (void (MainPage::*)(Platform::Object^,
                Windows::UI::Xaml::RoutedEventArgs^))&MainPage::resume);

            statusMessage->Visibility = Windows::UI::Xaml::Visibility::Visible;
            statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
                SolidColorBrush(Windows::UI::Colors::Green);
            statusMessage->Text = "Paused";
        }

    }
    catch (Platform::Exception^ exception)
    {
        playbackError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     resume
 *  @brief  resume the playback of the media file
 *
 *  @return
 *******************************************************************************
 */
void MainPage::resume()
{
    try
    {
        if (isPlaying)
        {
            video->Play();

            pauseResumeButton->Content = L"Pause";
            pauseResumeButton->Click -= pauseResumeEventRegToken;
            pauseResumeEventRegToken = pauseResumeButton->Click += ref new
                RoutedEventHandler(this, (void (MainPage::*)(Platform::Object^,
                Windows::UI::Xaml::RoutedEventArgs^))&MainPage::pause);

            statusMessage->Visibility = Windows::UI::Xaml::Visibility::Visible;
            statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
                SolidColorBrush(Windows::UI::Colors::Green);
            statusMessage->Text = "Resumed";
        }
    }
    catch (Platform::Exception^ exception)
    {
        playbackError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     stop
 *  @brief  stop the playback of the media file
 *
 *  @return
 *******************************************************************************
 */
void MainPage::stop()
{
    try
    {
        resume();
        video->Stop();
        isPlaying = false;
        propertySet = ref new PropertySet();

        statusMessage->Visibility = Windows::UI::Xaml::Visibility::Visible;
        statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
            SolidColorBrush(Windows::UI::Colors::Green);
        statusMessage->Text = "Stopped";
    }
    catch (Platform::Exception^ exception)
    {
        playbackError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     setRecommendedSettings
 *  @brief  Set the recommended settings for VQ
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::setRecommendedSettings()
{
    int height = video->NaturalVideoHeight;
    int width = video->NaturalVideoWidth;
    BOOL interlaceMode = TRUE;

    Windows::Foundation::Collections::PropertySet^ recommendedVqSettings =
        VqHelpers::GetRecommendedSettings(width, height, interlaceMode,
        deinterlaceMethod, AMFCMRequestType::AMF_CM_REALTIME);

    updateVqControls(recommendedVqSettings);
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    isVqSupported = (bool)(e->Parameter);
    vqSupportStatus->Visibility = isVqSupported ? Windows::UI::Xaml::
        Visibility::Collapsed : Windows::UI::Xaml::Visibility::Visible;

    recommendedButton->IsEnabled = isVqSupported;
    steadyVideoCheckBox->IsEnabled = isVqSupported;
    zoomSlider->IsEnabled = isVqSupported;
    delaySlider->IsEnabled = isVqSupported;
    strengthSlider->IsEnabled = isVqSupported;
    edgeEnhancementCheckBox->IsEnabled = isVqSupported;
    edgeEnhancementSlider->IsEnabled = isVqSupported;
    denoiseCheckBox->IsEnabled = isVqSupported;
    denoiseSlider->IsEnabled = isVqSupported;
    mosquitoCheckBox->IsEnabled = isVqSupported;
    mosquitoSlider->IsEnabled = isVqSupported;
    deblockingCheckBox->IsEnabled = isVqSupported;
    deblockingSlider->IsEnabled = isVqSupported;
    autoRadioButton->IsEnabled = isVqSupported;
    weaveRadioButton->IsEnabled = isVqSupported;
    bobRadioButton->IsEnabled = isVqSupported;
    adaptiveRadioButton->IsEnabled = isVqSupported;
    motionAdaptiveRadioButton->IsEnabled = isVqSupported;
    vectorAdaptiveRadioButton->IsEnabled = isVqSupported;
    brightnessSlider->IsEnabled = isVqSupported;
    contrastSlider->IsEnabled = isVqSupported;
    saturationSlider->IsEnabled = isVqSupported;
    tintSlider->IsEnabled = isVqSupported;
    colorVibranceCheckBox->IsEnabled = isVqSupported;
    colorVibranceSlider->IsEnabled = isVqSupported;
    fleshToneCorrectionCheckBox->IsEnabled = isVqSupported;
    fleshToneCorrectionSlider->IsEnabled = isVqSupported;
    brighterWhitesCheckBox->IsEnabled = isVqSupported;
    dynamicRangeCheckBox->IsEnabled = isVqSupported;
    fullRangeRadioButton->IsEnabled = isVqSupported;
    limitedRangeRadioButton->IsEnabled = isVqSupported;
    videoGammaCheckBox->IsEnabled = isVqSupported;
    videoGammaSlider->IsEnabled = isVqSupported;
    dynamicContrastCheckBox->IsEnabled = isVqSupported;
    pulldownDetectionCheckBox->IsEnabled = isVqSupported;
    falseContourReduceCheckBox->IsEnabled = isVqSupported;
    falseContourReduceSlider->IsEnabled = isVqSupported;
    demoModeCheckBox->IsEnabled = isVqSupported;
}