//
// Created by bkg2k on 17/04/23.
//
#pragma once

#include "guis/IGuiDownloaderUpdater.h"
#include "systems/BaseSystemDownloader.h"
#include "utils/network/Http.h"

enum class GenericDownloadingGameState
{
    Start,             // Init
    // Actions
    Downloading,       //!< Downloading games
    Extracting,        //!< Extracting
    UpdatingMetadata,  //!< Update metadata
    // Errors
    WriteOnlyShare,    //!< Share is write only!
    DownloadError,     //!< Error downloading file(s)
    InvalidUrlFormat   //!< Invalid Url Format. Has to be https://..../filename.ext
};

class GenericDownloader : public BaseSystemDownloader
                        , private ISyncMessageReceiver<GenericDownloadingGameState>
                        , private Http::IDownload
{
  public:
    GenericDownloader(SystemData& system, IGuiDownloaderUpdater& updater);
    GenericDownloader(SystemData& system, String url, IGuiDownloaderUpdater& updater);

    /*
     * Http::IDownload implementation
     */

    /*!
     * @brief Notify of download progress
     * @param http HTTP request
     * @param currentSize downloaded bytes
     * @param expectedSize total expected bytes
     */
    void DownloadProgress(const Http& http, long long currentSize, long long expectedSize) override;

  private:
    //! Game fetching URL
    static constexpr const char* sRepoBaseURL = "https://gitlab.com/recalbox/packages/game-providers/%s/-/archive/main/wasp4-main.zip";

    //! Http request object
    Http mRequest;

    //! Sync messager
    SyncMessageSender<GenericDownloadingGameState> mSender;

    //! Time reference
    DateTime mTimeReference;

    //! Wasm4 system reference
    SystemData& mSystem;

    //! File length
    long long mTotalSize;
    //! Downloaded length
    long long mCurrentSize;

    //! Extracted games
    int mGames;

    // source url
    String mUrl = "";

    /*!
     * @brief Receive synchronous code
     */
    void ReceiveSyncMessage(const GenericDownloadingGameState& code) override;

    /*
     * BaseSystemDownloader implementation
     */

    /*!
     * @brief Actually download & install
     */
    void DownloadAndInstall() override;

    /*!
     * @brief Called once when the process is complete
     * @param stopped true if the process has been stopped by calling MustExitAsap
     */
    void Completed(bool stopped) override;
};
