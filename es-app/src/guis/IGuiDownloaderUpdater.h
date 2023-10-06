//
// Created by bkg2k on 17/04/23.
//
#pragma once

#include <systems/SystemData.h>

/*!
 * @brief Allow downloader to update the UI
 */
class IGuiDownloaderUpdater
{
  public:
    //! Default required destructor
    virtual ~IGuiDownloaderUpdater() = default;

    /*!
     * @brief Update progress bar
     * @param value Current value
     * @param total Total value
     */
    virtual void UpdateProgressbar(long long value, long long total) = 0;

    /*!
     * @brief Update main update window text
     * @param text Text to update
     */
    virtual void UpdateMainText(const String& text) = 0;

    /*!
     * @brief Update title text
     * @param text Text to update
     */
    virtual void UpdateTitleText(const String& text) = 0;

    /*!
     * @brief Update ETA text (progressbar related)
     * @param text Text tu update
     */
    virtual void UpdateETAText(const String& text) = 0;

    /*!
     * @brief Notify the UI the download is complete
     * @param system System that needs to be refreshed
     * @param aborted true if the download has been cancelled by the user
     */
    virtual void DownloadComplete(SystemData& system, bool aborted) = 0;
};