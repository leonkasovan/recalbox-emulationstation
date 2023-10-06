//
// Created by bkg2k on 04/12/2019.
//
#pragma once

#include "ScrapeResult.h"

class INotifyScrapeResult
{
  public:
    //! Virtual destructor
    virtual ~INotifyScrapeResult() = default;

    /*!
     * @brief Notify a game has been scraped
     * @param index Game index
     * @param total Total game to scrape
     * @param result Result object
     * @param changedMetadata Changed metadata of result
     */
    virtual void GameResult(int index, int total, FileData* result, MetadataType changedMetadata) = 0;

    /*!
     * @brief Notify the caller of scraping is being aborted immediately
     * @param reason Reason of completion (ok/abort)
     * @param changedMetadata Changed metadata on all games
     */
    virtual void ScrapingComplete(ScrapeResult reason, MetadataType changedMetadata) = 0;
};
