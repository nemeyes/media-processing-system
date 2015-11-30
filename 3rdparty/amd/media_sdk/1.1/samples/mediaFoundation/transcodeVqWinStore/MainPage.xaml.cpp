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

using namespace TranscodeVqWinStore;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
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
{
    InitializeComponent();

    /**************************************************************************
     * Occurs when a Button is clicked                                        *
     * Example pickfile() occurs when we click pickfileButton,                *
     *         transcodeWithVQ() occurs when we click transcodeButton,        *
     *         transcodeCancel() occurs when we click cancelButton,           *
     **************************************************************************/
    openfileButton->Click += ref new RoutedEventHandler(this, &MainPage::
        pickfile);
    transcodeButton->Click += ref new RoutedEventHandler(this, &MainPage::
        transcodeWithVQ);
    cancelButton->Click += ref new RoutedEventHandler(this, &MainPage::
        transcodeCancel);

    /**************************************************************************
     * Creates an encoding profile of H.264 video                             *
     **************************************************************************/
    profile = MediaEncodingProfile::CreateMp4(VideoEncodingQuality::HD1080p);

    /**************************************************************************
     * Creates a new instance of MediaTranscoder class                        *
     **************************************************************************/
    transcoder = ref new MediaTranscoder();

    /**************************************************************************
     * Creates and initializes new instance of PropertySet                    *
     **************************************************************************/
    propertySetTranscode = ref new PropertySet();
    propertySetPreview = ref new PropertySet();

    propertySet = propertySetTranscode;

    updateFromControls();
    updateScale();

    propertySet = propertySetPreview;
    updateFromControls();

    /**************************************************************************
     * Adds the AMF effect with the configuration properties and indicates    *
     * whether the effect is required or not                                  *
     **************************************************************************/
    transcoder->AddVideoEffect("mftvqLib.AMFVideoTransform", true, propertySet);

    /**************************************************************************
     * Disabling the transcode button and cancel button until file is picked  *
     **************************************************************************/
    transcodeButton->IsEnabled = false;
    cancelButton->IsEnabled = false;

    /**************************************************************************/
    /* Intialize the files                                                    */
    /**************************************************************************/
    inputFile = nullptr;
    outputFile = nullptr;

    /**************************************************************************
     * Construct a cancellation_token_source oject, it has the ability to     *
     * cancel an operation                                                    *
     **************************************************************************/
    cts = cancellation_token_source();

    /**************************************************************************
     * Initialize and handle UI controls                                      *
     **************************************************************************/
    initDefaults();
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
void MainPage::pickfile(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        ApplicationViewState currentState = ApplicationView::Value;
        if(currentState == ApplicationViewState::Snapped && !ApplicationView::
            TryUnsnap())
        {
            transcodeError("Cannot pick files while application is in snapped  \
                view");
        }
        else
        {
            /******************************************************************
             * Choose and Open file, the file open picker dispaly the file    *
             * types of ".wmv", ".mp4", ".mpg", ".mpeg", ".asf"               *
             * file open picker looks for files from "VideoLibrary"           *
             ******************************************************************/
            FileOpenPicker^ picker = ref new FileOpenPicker();
            picker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
            picker->FileTypeFilter->Append(".wmv");
            picker->FileTypeFilter->Append(".mp4");
            picker->FileTypeFilter->Append(".mpg");
            picker->FileTypeFilter->Append(".mpeg");
            picker->FileTypeFilter->Append(".asf");
            picker->FileTypeFilter->Append(".avi");

            /******************************************************************
             * Creates a task which shows the file picker so that user can    *
             * pick one file, play it and show the file path.                 *
             * Enable transcode button, trancode cancel button, AMF video     *
             * effect buttons, radio buttons, sliders and etc                 *
             ******************************************************************/
            create_task(picker->PickSingleFileAsync()).then(
                            [this](StorageFile^ sourceFile)
            {
                try
                {
                    inputFile = sourceFile;
                    /**********************************************************
                     * Play the input file                                    *
                     **********************************************************/
                    playFile(inputFile);
                    if(inputFile != nullptr)
                    {
                        return sourceFile->OpenAsync(FileAccessMode::Read);
                    }
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }

                cancel_current_task();
            }).then([this](IRandomAccessStream^ inputStream)
            {
                try
                {
                    /**********************************************************
                     * Show the input file path                               *
                     **********************************************************/
                    inputMsg->Foreground = ref new Windows::UI::Xaml::Media::
                        SolidColorBrush(Windows::UI::Colors::White);
                    inputMsg->Text = inputFile->Path;

                    /**********************************************************
                     * Enable transcode button, AMF video effect buttons,     *
                     * radio buttons, sliders and etc                         *
                     **********************************************************/
                    enableButtons();
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }
            });
        }
    }
    catch(Exception^ exception)
    {
        transcodeError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     transcodeWithVQ
 *  @brief  Occurs when transcodeButton clicked
 *
 *  @param[in] sender:
 *  @param[in] e: state infromation and eavent data of routed event
 *
 *  @return
 *******************************************************************************
 */
void MainPage::transcodeWithVQ(Object^ sender, RoutedEventArgs^ e)
{
    /**************************************************************************
     * Disable transcode button, trancode cancel button, AMF video effect     *
     * buttons, radio buttons, sliders and etc                                *
     **************************************************************************/
    disableButtons();

    /**************************************************************************
     * pickup the latest changes                                              *
     **************************************************************************/
    propertySet = propertySetTranscode;
    updateFromControls();
    updateScale();

    propertySet = propertySetPreview;

    /**************************************************************************
     * User can change bitrate, gopSize, qualityVsspeed                       *
     **************************************************************************/
    changeEncoderParameters();
    statusMessage->Text = "";

    try
    {
        if (inputFile != nullptr)
        {
            auto videoLibrary = KnownFolders::VideosLibrary;
            FileSavePicker^ savePicker = ref new FileSavePicker();
            savePicker->SuggestedStartLocation = \
                PickerLocationId::VideosLibrary;

            auto extensions = ref new Platform::Collections::Vector<String^>();
            extensions->Append(".mp4");
            savePicker->FileTypeChoices->Insert("Video", extensions);
            savePicker->SuggestedFileName = "TranscodeWithVQ";

            /******************************************************************
             * Create a Task for creating a new unique file in current folder *
             ******************************************************************/
            create_task(savePicker->PickSaveFileAsync(), cts.get_token()).then
                ([this](StorageFile^ destinationFile)
            {
                try
                {
                    if(destinationFile != nullptr)
                    {
                        outputFile = destinationFile;
                        outputMsg->Foreground = ref new Windows::UI::Xaml::
                            Media::SolidColorBrush(Windows::UI::Colors::White);
                        outputMsg->Text = outputFile->Path;

                        /******************************************************
                         * Performs an asynchronous transcode deferral        *
                         * operation on the source file and sends the         *
                         * converted media data to the destination file.      *
                         ******************************************************/
                        if(isVqSupported == false)
                        {
                            transcoder->ClearEffects();
                        }
                        return transcoder->PrepareFileTranscodeAsync(inputFile,
                            outputFile, profile);
                    }
                    else
                    {
                        cts.cancel();
                        cts = cancellation_token_source();
                    }
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }

                cancel_current_task();
            }).then(
            [this](PrepareTranscodeResult^ transcode)
            {
                try
                {
                    if(transcode->CanTranscode)
                    {
                        statusMessage->Foreground = ref new Windows::UI::Xaml::
                            Media::SolidColorBrush(Windows::UI::Colors::
                            MediumSpringGreen);
                        statusMessage->Text = "Transcoding...";
                        setCancelButton(true);

                        /******************************************************
                         * Creates an object to perform an asynchronous media *
                         * transcode operation on media data.                 *
                         ******************************************************/
                        Windows::Foundation::IAsyncActionWithProgress<double>^
                            transcodeOp = transcode->TranscodeAsync();
                        /******************************************************
                         * Stop preview playback during transcoding.          *
                         ******************************************************/
                        preview->Stop();

                        transcodeOp->Progress = ref new
                            AsyncActionProgressHandler<double>(
                            [this](IAsyncActionWithProgress<double>^ asyncInfo,
                            double percent)
                        {
                            /**************************************************
                             * Shows transcoding progress in percent          *
                             **************************************************/
                            transcodeProgress(asyncInfo, percent);
                        }, Platform::CallbackContext::Same);

                        return transcodeOp;
                    }
                    else
                    {
                        transcodeFailure(transcode->FailureReason);
                    }
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }

                cancel_current_task();
            }).then(
            [this](task<void> transcodeTask)
            {
                try
                {
                    transcodeTask.get();
                    statusMessage->Foreground = ref new Windows::UI::Xaml::
                        Media::SolidColorBrush(Windows::UI::Colors::
                        MediumSpringGreen);
                    statusMessage->Text = "Transcode Completed. ";
                }
                catch (task_canceled&)
                {
                    statusMessage->Foreground = ref new Windows::UI::Xaml::
                        Media::SolidColorBrush(Windows::UI::Colors::Red);
                    statusMessage->Text += "Transcode Cancelled. ";
                }
                catch(Exception^ exception)
                {
                    transcodeError(exception->Message);
                }

                /**************************************************************
                 * Enable transcode button, AMF video effect buttons,         *
                 * radio buttons, sliders and etc; Disable cancel button;     *
                 * start play the input file                                  *
                 **************************************************************/
                enableButtons();
                setCancelButton(false);
                preview->Play();
            });
        }
    }
    catch (Platform::Exception^ exception)
    {
        transcodeError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     transcodeCancel
 *  @brief  Occurs when transcodeCancelButton clicked
 *
 *  @param[in] sender:
 *  @param[in] e: state infromation and eavent data of routed event
 *
 *  @return
 *******************************************************************************
 */
void MainPage::transcodeCancel(Object^ sender, Windows::UI::Xaml::
    RoutedEventArgs^ e)
{
    try
    {
        statusMessage->Text = "";
        cts.cancel();
        cts = cancellation_token_source();

        if(outputFile != nullptr)
        {
            /******************************************************************
             * Create a task for deleting current file                        *
             ******************************************************************/
            create_task(outputFile->DeleteAsync()).then(
                            [this](task<void> deleteTask)
            {
                try
                {
                    deleteTask.get();
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }
            });
        }
    }
    catch (Platform::Exception^ exception)
    {
        transcodeError(exception->Message);
    }
}

/**
 *******************************************************************************
 *  @fn     transcodeFailure
 *  @brief  Shows the transcode failure reasons
 *
 *  @param[in] reason: transcode failure reason
 *
 *  @return
 *******************************************************************************
 */
void MainPage::transcodeFailure(TranscodeFailureReason reason)
{
    try
    {
        if(outputFile != nullptr)
        {
            /******************************************************************
             * Deletes the output file                                        *
             ******************************************************************/
            create_task(outputFile->DeleteAsync()).then(
                            [this](task<void> deleteTask)
            {
                try
                {
                    deleteTask.get();
                }
                catch (Platform::Exception^ exception)
                {
                    transcodeError(exception->Message);
                }
            });
        }
    }
    catch(Platform::Exception^ exception)
    {
        transcodeError(exception->Message);
    }

    /**************************************************************************
     * Updating transcoding failure reasons                                   *
     **************************************************************************/
    switch (reason)
    {
    case TranscodeFailureReason::CodecNotFound:
        transcodeError("Codec not found.");
        break;
    case TranscodeFailureReason::InvalidProfile:
        transcodeError("Invalid profile.");
        break;
    default:
        transcodeError("Unknown failure.");
    }
}

/**
 *******************************************************************************
 *  @fn     transcodeError
 *  @brief  Shows the transcode error
 *
 *  @param[in] Error: Transcoding error
 *
 *  @return
 *******************************************************************************
 */
void MainPage::transcodeError(Platform::String^ error)
{
    /**************************************************************************
     * Updating transcoding errors                                            *
     **************************************************************************/
    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
        SolidColorBrush(Windows::UI::Colors::Red);
    statusMessage->Text = error + " ";

    /**************************************************************************
     * Enable transcode button, AMF video effect buttons, radio buttons,      *
     * sliders and etc; Disable cancel button;                                *
     **************************************************************************/
    enableButtons();
    setCancelButton(false);
}

/**
 *******************************************************************************
 *  @fn     transcodeProgress
 *  @brief  Shows the transcoding progress
 *
 *  @param[in] asyncInfo: Asynchronous action that includes progress updates
 *  @param[in] percent: shows how much percetange of transcoding was done
 *
 *  @return
 *******************************************************************************
 */
void MainPage::transcodeProgress(IAsyncActionWithProgress<double>^ asyncInfo,
    double percent)
{
    /**************************************************************************
     * Updating transcoding progress                                          *
     **************************************************************************/
    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::
        SolidColorBrush(Windows::UI::Colors::MediumSpringGreen);
    statusMessage->Text = "Transcoding... Progress:  " + ((int)percent).
        ToString() + "%";
}

/**
 *******************************************************************************
 *  @fn     setCancelButton
 *  @brief  Enables the cancel button
 *
 *  @param[in] isEnabled: flag to enable/disable
 *
 *  @return
 *******************************************************************************
 */
void MainPage::setCancelButton(bool isEnabled)
{
    cancelButton->IsEnabled = isEnabled;
}

/**
 *******************************************************************************
 *  @fn     enableButtons
 *  @brief  Enables various button, slider, radiobutton, textboxes
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::enableButtons()
{
    openfileButton->IsEnabled = true;
    transcodeButton->IsEnabled = true;
    transcodeProfileCombo->IsEnabled = true;
    gopSizeTextBox->IsEnabled = true;
    bitrateSlider->IsEnabled = true;
    qualityVsSpeedSlider->IsEnabled = true;
	bFrameCountCombo->IsEnabled = true;
}

/**
 *******************************************************************************
 *  @fn     disableButtons
 *  @brief  Disables various button, slider, radiobutton, textboxes
 *
 *  @param
 *
 *  @return
 *******************************************************************************
 */
void MainPage::disableButtons()
{
    openfileButton->IsEnabled = false;
    transcodeButton->IsEnabled = false;
    transcodeProfileCombo->IsEnabled = false;
    gopSizeTextBox->IsEnabled = false;
    bitrateSlider->IsEnabled = false;
    qualityVsSpeedSlider->IsEnabled = false;
	bFrameCountCombo->IsEnabled = false;
}

/**
 *******************************************************************************
 *  @fn     playFile
 *  @brief  Playing a media file with AMF VQ effects
 *
 *  @param[in] MediaFile: media file to be played
 *
 *  @return
 *******************************************************************************
 */
void MainPage::playFile(Windows::Storage::StorageFile^ MediaFile)
{
    try
    {
        /**********************************************************************
         * Open input media file                                              *
         **********************************************************************/
        create_task(MediaFile->OpenAsync(FileAccessMode::Read)).then(
                        [&, this, MediaFile](IRandomAccessStream^ outputStream)
        {
            try
            {
                /**************************************************************
                 * Sets the source property using the supplied stream, add VQ *
                 * effect, Play the stream                                    *
                 **************************************************************/
                preview->RemoveAllEffects();
                updateFromControls();
                if(isVqSupported)
                {
                    preview->AddVideoEffect("mftvqLib.AMFVideoTransform", true,
                        propertySetPreview);
                }
                preview->SetSource(outputStream, MediaFile->ContentType);
                preview->Play();

            }
            catch (Platform::Exception^ exception)
            {
                transcodeError(exception->Message);
            }
        });
    }
    catch (Exception^ exception)
    {
        transcodeError(exception->Message);
    }
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
        10);
    initSlider(denoiseSlider, &MainPage::updateFromControls, 1, 100, 64);
    initSlider(mosquitoSlider, &MainPage::updateFromControls, 1, 100, 50);
    initSlider(deblockingSlider, &MainPage::updateFromControls, 1, 100, 50);
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
    initSlider(falseContourReduceSlider, &MainPage::updateFromControls, 0, 100,
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
    initCheckBox( demoModeCheckBox,  &MainPage::updateFromControls, false );

    updateFromControls();
}

/**
 *******************************************************************************
 *  @fn     updateFromControls
 *  @brief  Update property set from UI controls
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
 *  @fn     updateScale
 *  @brief  Update scaling parameters
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::updateScale()
{
    propertySet->Insert(AMF_EFFECT_SCALE_WIDTH, profile->Video->Width);
    propertySet->Insert(AMF_EFFECT_SCALE_HEIGHT, profile->Video->Height);
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
 *  @fn     updateVqControls
 *  @brief  Update VQ params from property set
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
        auto isEnabled = [&vqPropertySet](const wchar_t* featureName)
        {
            Platform::String^ name = ref new Platform::String(featureName);

            bool hasKey = vqPropertySet->HasKey(name);

            Windows::Foundation::IPropertyValue^ val = dynamic_cast<Windows::
                Foundation::IPropertyValue^>(vqPropertySet->Lookup(name));

            bool enabled = val->GetBoolean();

            return hasKey && enabled;
        };

        auto updateSlider = [&vqPropertySet](
            bool enabled,
            const wchar_t* parameterName,
            Windows::UI::Xaml::Controls::Slider^ slider)
        {
            slider->IsEnabled = enabled;

            Platform::String^ name = ref new Platform::String(parameterName);
            if (vqPropertySet->HasKey(name))
            {
                Windows::Foundation::IPropertyValue^ value =
                    dynamic_cast<Windows:: Foundation::IPropertyValue^>(
                        vqPropertySet->Lookup(name));
                slider->Value = value->GetDouble();
            }
        };

        steadyVideoCheckBox->IsChecked = isEnabled(AMF_EFFECT_STEADY_VIDEO);
        updateSlider(steadyVideoCheckBox->IsChecked->Value,
            AMF_EFFECT_STEADY_VIDEO_ZOOM, zoomSlider);
        updateSlider(steadyVideoCheckBox->IsChecked->Value,
            AMF_EFFECT_STEADY_VIDEO_DELAY,  delaySlider);
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

        dynamicContrastCheckBox->IsChecked
            = isEnabled(AMF_EFFECT_DYNAMIC_CONTRAST);

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
 *  @fn     changeEncoderParameters
 *  @brief  setting up the encode params - bitrate, qualityVsSpeed, gopSize
 *
 *  @param[]
 *
 *  @return
 *******************************************************************************
 */
void MainPage::changeEncoderParameters()
{
    unsigned int bitrate = (int) bitrateSlider->Value;
    unsigned int qualityVsSpeed = (int) qualityVsSpeedSlider->Value;
    unsigned int gopSize = _wtoi(gopSizeTextBox->Text->Data());
    if (gopSize < 0 || gopSize >= INT32_MAX)
    {
        gopSize = 256;
    }

	unsigned int bFrameCount;
	switch(bFrameCountCombo->SelectedIndex)
	{
	case 0:
		bFrameCount = 0;
		break;
	case 1:
		bFrameCount = 1;
		break;
	case 2:
		bFrameCount = 2;
		break;
	case 3:
		bFrameCount = 3;
		break;
	}

    profile = nullptr;

    VideoEncodingQuality videoEncodingProfile = VideoEncodingQuality::Wvga;

    switch (transcodeProfileCombo->SelectedIndex)
    {
    case 0:
        videoEncodingProfile = VideoEncodingQuality::HD1080p;
        break;
    case 1:
        videoEncodingProfile = VideoEncodingQuality::HD720p;
        break;
    case 2:
        videoEncodingProfile = VideoEncodingQuality::Wvga;
        break;
    case 3:
        videoEncodingProfile = VideoEncodingQuality::Ntsc;
        break;
    case 4:
        videoEncodingProfile = VideoEncodingQuality::Pal;
        break;
    case 5:
        videoEncodingProfile = VideoEncodingQuality::Vga;
        break;
    case 6:
        videoEncodingProfile = VideoEncodingQuality::Qvga;
        break;
    }

    profile = MediaEncodingProfile::CreateMp4(videoEncodingProfile);

    auto videoProperties = profile->Video->Properties;

    videoProperties->Insert(CODECAPI_AVEncCommonMeanBitRate, bitrate);
    videoProperties->Insert(CODECAPI_AVEncCommonQualityVsSpeed, qualityVsSpeed);
    videoProperties->Insert(CODECAPI_AVEncMPVGOPSize, gopSize);
	videoProperties->Insert(CODECAPI_AVEncMPVDefaultBPictureCount, bFrameCount);
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
void MainPage::setRecommendedSettings(Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    int height = 1920;
    int width = 1080;
    BOOL interlaceMode = TRUE;

    Windows::Foundation::Collections::PropertySet^ recommendedVqSettings =
        VqHelpers::GetRecommendedSettings(width, height, interlaceMode,
        deinterlaceMethod, AMFCMRequestType::AMF_CM_NONREALTIME);

    updateVqControls(recommendedVqSettings);
}

/**
 *******************************************************************************
 *  @fn     OnNavigatedTo
 *  @brief  Invoked when this page is about to be displayed in a Frame.
 *
 *  @param[in] e: Event data that describes how this page was reached.
 *                The Parameter property is typically used to configure the page
 *
 *  @return void: "e"
 *******************************************************************************
 */
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
