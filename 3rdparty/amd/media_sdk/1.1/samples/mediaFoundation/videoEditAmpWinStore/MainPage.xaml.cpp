/*******************************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1              Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
2              Redistributions in binary form must reproduce the above copyright notice, 
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
* @file <MainPage.xaml.cpp>
*                                       
* @brief The implementation code-behind files for the Transcode Application and
*        MianPage classes. In this file we can add event handlers and other 
*        custom program logic that's related to this page. variables in the Page
*        are in scope only in that page.
*         
********************************************************************************
*/

#include "pch.h"
#include "MainPage.xaml.h"
#include <Codecapi.h>

using namespace videoEditAmpWinStore;

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
 ********************************************************************************
 */
MainPage::MainPage()
{
    InitializeComponent();
    /**************************************************************************
    *  Occurs when a Button is clicked                                        *
    *  Example pickfile() occurs when we click pickfileButton,                *
    *          transcodeWithVQ() occurs when we click transcodeButton,        *
    *          transcodeCancel() occurs when we click cancelButton,           *
    **************************************************************************/
    pickfileButton->Click += ref new RoutedEventHandler(this, &MainPage::pickfile);
    transcodeButton->Click += ref new RoutedEventHandler(this, &MainPage::transcodeWithAmp);
    cancelButton->Click += ref new RoutedEventHandler(this, &MainPage::transcodeCancel);

    /**************************************************************************
    *  Creates an encoding mProfile of H.264 video                            *
    **************************************************************************/
    mProfile = MediaEncodingProfile::CreateMp4(VideoEncodingQuality::Wvga);

    /**************************************************************************
    * Creates a new instance of MediaTranscoder class                         *
    **************************************************************************/
    mTranscoder = ref new MediaTranscoder();   

    /**************************************************************************
    *  Creates and initializes new instance of PropertySet                    *
    **************************************************************************/
    mPropertySet = ref new PropertySet();

    /***************************************************************************
    *  Adds the resizer MFT with the configuration properties and indicates    *
    *  whether the effect is required or not                                   *
    ***************************************************************************/
    mTranscoder->AddVideoEffect("ampTransformExt.ampTransform", true, mPropertySet);

    /**************************************************************************
    *  Disabling the transcode button and cancel button until file is picked  *
    **************************************************************************/
    transcodeButton->IsEnabled = false;
    cancelButton->IsEnabled = false;

    /**************************************************************************
    *  Intialize the files                                                    *
    **************************************************************************/
    mInputFile = nullptr;
    mOutputFile = nullptr;

    /**************************************************************************
    *  Construct a cancellation_token_source oject, it has the ability to     *
    *  cancel an operation                                                    *
    **************************************************************************/
    mCts = cancellation_token_source();

    /**************************************************************************
    *  Initialize and handle UI controls                                      *
    **************************************************************************/
    initDefaults();
}

/** 
 *******************************************************************************
 *  @fn     OnNavigatedTo
 *  @brief  Invoked when this page is about to be displayed in a Frame.
 *           
 *  @param[in] e: Event data that describes how this page was reached.  The Parameter
 *            property is typically used to configure the page.</param>
 *          
 *  @return void: "e"
 *******************************************************************************
 */
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    (void) e; // Unused parameter
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
        if(currentState == ApplicationViewState::Snapped && !ApplicationView::TryUnsnap())
        {
            transcodeError("Cannot pick files while application is in snapped view");
        }
        else
        {
            /******************************************************************
            *  Choose and Open file, the file open picker dispaly the file    *
            *  types of ".wmv", ".mp4", ".mpg", ".mpeg", ".asf"               *
            *  file open picker looks for files from "VideoLibrary"           *
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
            *  Creates a task which shows the file picker so that user can    *
            *  pick one file, play it and show the file path.                 *
            *  Enable transcode button, trancode cancel button, AMF video     *
            *  effect buttons, radio buttons, sliders and etc                 *
            ******************************************************************/
            create_task(picker->PickSingleFileAsync()).then(
                [this](StorageFile^ sourceFile)
            {
                if(sourceFile)
                {
                    try
                    {
                        mInputFile = sourceFile;
                        /*******************************************************
                        *  Play the input file                                 *
                        *******************************************************/
                        playFile(mInputFile);
                        if(mInputFile != nullptr)
                        {
                            return sourceFile->OpenAsync(FileAccessMode::Read);
                        }
                    }
                    catch (Platform::Exception^ exception)
                    {
                        transcodeError(exception->Message);
                    }
                }

                cancel_current_task();
            }).then(
                [this](IRandomAccessStream^ inputStream)
            {
                try
                {
                    /**********************************************************
                    *  Show the input file path                               *
                    **********************************************************/
                    inputMsg->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
                    inputMsg->Text = mInputFile->Path;

                    /**********************************************************
                    *  Enable transcode button, AMF video effect buttons,     *
                    *  radio buttons, sliders and etc                         *
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
void MainPage::transcodeWithAmp(Object^ sender, RoutedEventArgs^ e)
{
    /**************************************************************************
    *  Disable transcode button, trancode cancel button, AMF video effect     *
    *  buttons, radio buttons, sliders and etc                                *
    **************************************************************************/
    disableButtons();

    /**************************************************************************
    *  User can change bitrate, gopSize, qualityVsspeed                       *
    **************************************************************************/
    changeEncoderParameters();
    statusMessage->Text = "";

    try
    {
        if (mInputFile != nullptr)
        {
            auto videoLibrary = KnownFolders::VideosLibrary;
            FileSavePicker^ savePicker = ref new FileSavePicker();
            savePicker->SuggestedStartLocation = \
                PickerLocationId::VideosLibrary;

            auto extensions = ref new Platform::Collections::Vector<String^>();
            extensions->Append(".mp4");
            savePicker->FileTypeChoices->Insert("Video", extensions);
            savePicker->SuggestedFileName = "VideoEditWithAmp";

            /******************************************************************
             * Create a Task for creating a new unique file in current folder *
             ******************************************************************/
            create_task(savePicker->PickSaveFileAsync(), mCts.get_token()).then
                ([this](StorageFile^ destinationFile)
            {
                try
                {
                    if(destinationFile != nullptr)
                    {
                        mOutputFile = destinationFile;
                        outputMsg->Foreground = ref new Windows::UI::Xaml::Media::
                                                    SolidColorBrush(Windows::UI::Colors::White);
                        outputMsg->Text = mOutputFile->Path;
                        /**********************************************************
                        *  Performs an asynchronous transcode deferral operation  *
                        *  on the source file and sends the converted media data  *
                        *  to the destination file.                               *
                        **********************************************************/
                        return mTranscoder->PrepareFileTranscodeAsync(mInputFile, mOutputFile, mProfile);
                    }
                    else
                    {
                        mCts.cancel();
                        mCts = cancellation_token_source();
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
                        statusMessage->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::MediumSpringGreen);
                        statusMessage->Text = "Transcoding...";
                        setCancelButton(true);

                        /******************************************************
                        *  Creates an object to perform an asynchronous media *
                        *  transcode operation on media data.                 *
                        ******************************************************/
                        Windows::Foundation::IAsyncActionWithProgress<double>^ transcodeOp = transcode->TranscodeAsync();
                        /******************************************************
                        *  Stop preview playback during transcoding.          *
                        ******************************************************/
                        preview->Stop();

                        transcodeOp->Progress = ref new AsyncActionProgressHandler<double>(
                        [this](IAsyncActionWithProgress<double>^ asyncInfo, double percent)
                        {
                            /**************************************************
                            *  Shows transcoding progress in percent          *
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
                    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::MediumSpringGreen);
                    statusMessage->Text = "Transcode Completed. ";
                }
                catch (task_canceled&)
                {
                    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red);
                    statusMessage->Text += "Transcode Cancelled. ";
                }
                catch(Exception^ exception)
                {
                    transcodeError(exception->Message);
                }

                /**********************************************************************
                *  Enable transcode button, AMF video effect buttons, radio buttons,  *
                *  sliders and etc; Disable cancel button; start play the input file  *
                **********************************************************************/
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
void MainPage::transcodeCancel(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        statusMessage->Text = "";
        mCts.cancel();
        mCts = cancellation_token_source();

        if(mOutputFile != nullptr)
        {
            /******************************************************************
            *  Create a task for deleting current file                        *
            ******************************************************************/
            create_task(mOutputFile->DeleteAsync()).then(
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
        if(mOutputFile != nullptr)
        {
            /******************************************************************
            *  Deletes the output file                                        *
            ******************************************************************/
            create_task(mOutputFile->DeleteAsync()).then(
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
    *  Updating transcoding failure reasons                                   *
    **************************************************************************/
    switch (reason)
    {
    case TranscodeFailureReason::CodecNotFound:
        transcodeError("Codec not found.");
        break;
    case TranscodeFailureReason::InvalidProfile:
        transcodeError("Invalid mProfile.");
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
    *  Updating transcoding errors                                            *
    **************************************************************************/
    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red);
    statusMessage->Text = error + " ";

    /**************************************************************************
    *  Enable transcode button, AMF video effect buttons, radio buttons,      *
    *  sliders and etc; Disable cancel button;                                *
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
    *  Updating transcoding progress                                          *
    **************************************************************************/
    statusMessage->Foreground = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::MediumSpringGreen);
    statusMessage->Text = "Transcoding... Progress:  " + ((int)percent).ToString() + "%";
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
    pickfileButton->IsEnabled = true;
    transcodeButton->IsEnabled = true;
    transcodeProfileCombo->IsEnabled = true;
    bitrateSlider->IsEnabled = true;
    gopSizeTextBox->IsEnabled = true;
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
    pickfileButton->IsEnabled = false;
    transcodeButton->IsEnabled = false;
    transcodeProfileCombo->IsEnabled = false;
    bitrateSlider->IsEnabled = false;
    gopSizeTextBox->IsEnabled = false;
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
        *  Open input media file                                              *
        **********************************************************************/
        create_task(MediaFile->OpenAsync(FileAccessMode::Read)).then(
            [&, this, MediaFile](IRandomAccessStream^ outputStream)
        {
            try
            {
                /**************************************************************
                *  Sets the source property using the supplied stream, add VQ *
                *  effect, Play the stream                                    *
                **************************************************************/
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
    unsigned int bitrate        = (int)bitrateSlider->Value;
    unsigned int qualityVsSpeed = (int)qualityVsSpeedSlider->Value;
    unsigned int gopSize        = _wtoi(gopSizeTextBox->Text->Data());
    if(gopSize < 0 || gopSize >= INT32_MAX)
    {
        gopSize = 256;
    }

    unsigned int bFrameCount;
    switch (bFrameCountCombo->SelectedIndex)
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

    mProfile = nullptr;

    VideoEncodingQuality videoEncodingProfile = VideoEncodingQuality::Wvga;

    switch(transcodeProfileCombo->SelectedIndex)
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

    mProfile = MediaEncodingProfile::CreateMp4(videoEncodingProfile);

    auto videoProperties = mProfile->Video->Properties;

    videoProperties->Insert(CODECAPI_AVEncCommonMeanBitRate, bitrate);
    videoProperties->Insert(CODECAPI_AVEncCommonQualityVsSpeed, qualityVsSpeed);
    videoProperties->Insert(CODECAPI_AVEncMPVGOPSize, gopSize);
    videoProperties->Insert(CODECAPI_AVEncMPVDefaultBPictureCount, bFrameCount);
}