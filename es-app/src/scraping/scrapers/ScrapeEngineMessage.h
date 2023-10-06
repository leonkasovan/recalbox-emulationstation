//
// Created by bkg2k on 27/07/22.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <games/MetadataType.h>

class FileData;

/*!
 * @brief Simple synchronized message for internal scraper use
 */

struct ScrapeEngineMessage
{
  //! Game reference
  FileData* mGame;
  //! Metadata changed
  MetadataType mMetadata;
  //! Scraping result
  ScrapeResult mResult;
};
