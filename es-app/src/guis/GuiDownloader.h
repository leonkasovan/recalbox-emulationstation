//
// Created by bkg2k on 16/04/23.
//
#pragma once

#include "guis/Gui.h"
#include "utils/network/HttpClient.h"
#include "components/TextComponent.h"
#include "components/ProgressBarComponent.h"
#include "components/ComponentGrid.h"
#include "systems/DownloaderManager.h"

class GuiDownloader : public Gui
                    , IGuiDownloaderUpdater
{
  public:

    /*!
     * @brief Constructor
     * @param window Window manager
     * @param system Target wasm4 system
     */
    GuiDownloader(WindowManager& window, SystemData& system, SystemManager& systemManager);

    /*!
     * @brief Destructor
     * Update System view
     */
    ~GuiDownloader() override {}

    /*!
     * @brief Check cancel bouton
     * @param event Event to process
     * @return True/false
     */
    bool ProcessInput(const InputCompactEvent& event) override;

    /*!
     * @brief Override helper texts
     * @param help Help bar
     * @return
     */
    bool getHelpPrompts(Help& help) override;

    /*
     * IGuiDownloadUpdater implementation
     */

    /*!
     * @brief Update progress bar
     * @param value Current value
     * @param total Total value
     */
    void UpdateProgressbar(long long value, long long total) override;

    /*!
     * @brief Update main update window text
     * @param text Text to update
     */
    void UpdateMainText(const String& text) override;

    /*!
     * @brief Update title text
     * @param text Text to update
     */
    void UpdateTitleText(const String& text) override;

    /*!
     * @brief Update ETA text (progressbar related)
     * @param text Text tu update
     */
    void UpdateETAText(const String& text) override;

    /*!
     * @brief Notify the UI the download is complete
     * @param system System that needs to be refreshed
     * @param aborted true if the download has been cancelled by the user
     */
    void DownloadComplete(SystemData& system, bool aborted) override;

  private:
    //! Download manager private instance
    DownloaderManager mDownloadManager;
    //! Active downloader
    BaseSystemDownloader* mDownloader;

    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mText;
    std::shared_ptr<ProgressBarComponent> mBar;
    std::shared_ptr<TextComponent> mEta;

    // SystemManager reference
    SystemManager& mSystemManager;
};
