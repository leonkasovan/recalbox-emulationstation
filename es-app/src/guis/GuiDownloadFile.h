//
// Created by Dhani Novan on 27/02/24.
//
#pragma once

#include <components/ComponentList.h>
#include "guis/Gui.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "components/ProgressBarComponent.h"
#include <recalbox/RecalboxSystem.h>
#include <themes/MenuThemeData.h>
#include <utils/os/system/Thread.h>
#include <utils/sync/SyncMessageSender.h>
#include "utils/network/HttpClient.h"

class GuiDownloadFile: public Gui
                       , private Thread
                       , private ISyncMessageReceiver<int>
                       , private HttpClient::IDownload
{
  public:
    // GuiDownloadFile(WindowManager& window, const String& Url);
    GuiDownloadFile(WindowManager& window, const String& Url, const String& system="");

    ~GuiDownloadFile() override;

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

  private:
    static constexpr const char* sDownloadFolder = "/recalbox/share/roms/";

    /*!
     * @brief Receive synchronous code
     */
    void ReceiveSyncMessage(int code) override;

    /*
     * Thread Implementation
     */

    /*!
     * @brief Main thread routine
     */
    void Run() override;

    /*
     * Http::IDownload implementation
     */

    /*!
     * @brief Notify of download progress
     * @param http HTTP request
     * @param currentSize downloaded bytes
     * @param expectedSize total expected bytes
     */
    void DownloadProgress(const HttpClient& http, long long currentSize, long long expectedSize);

    //! Http request objects
    HttpClient mFileRequest;

    //! Url to download
    String mUrl;
    
    // target system in which rom to be installed
    String mSystem;

    // texts
    String mDownloadedSize;
    String mError;
    String mMessage;
    Path destination;
    String destinationFileName;

    //! Time reference
    DateTime mTimeReference;

    //! File length
    long long mTotalSize;
    //! Downloaded length
    long long mCurrentSize;

    SyncMessageSender<int> mSender;
    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mText;
    std::shared_ptr<ProgressBarComponent> mBar;
    std::shared_ptr<TextComponent> mFooter;
};
