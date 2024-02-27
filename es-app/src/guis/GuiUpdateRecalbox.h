//
// Created by xizor on 20/05/18.
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
#include "utils/network/HttpUnxzUntar.h"

#define PRE_UPGRADE_SCRIPT "/boot/update/pre-upgrade.sh"

class GuiUpdateRecalbox: public Gui
                       , private Thread
                       , private ISyncMessageReceiver<int>
                       , private HttpClient::IDownload
{
  public:
    GuiUpdateRecalbox(WindowManager& window, const String& tarUrl, const String& imageUrl, const String& sha1Url, const String& newVersion);

    ~GuiUpdateRecalbox() override;

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

  private:
    static constexpr const char* sDownloadFolder = "/boot/update";

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
    HttpUnxzUntar mTarRequest;
    HttpClient mImgRequest;

    //! Tar Url to download and decompress
    String mTarUrl;
    //! Image Url to download
    String mImageUrl;
    //! Sha1 Url to download
    String mSha1Url;
    //! New version
    String mNewVersion;
    // texts
    String mRebootIn;
    String mError;

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
    std::shared_ptr<TextComponent> mEta;
};
