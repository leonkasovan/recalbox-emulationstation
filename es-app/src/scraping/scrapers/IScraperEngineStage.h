//
// Created by bkg2k on 19/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <games/MetadataType.h>

// Forward declaration
class FileData;

class IScraperEngineStage
{
  public:
    //! Virtual destructor
    virtual ~IScraperEngineStage() = default;

    //! Stages
    enum class Stage
    {
      Text,      //!< All text info have been downloaded
      Images,    //!< All images have been downloaded
      Video,     //!< All video have been downloaded
      Extra,     //!< All extra media have been downloaded
      Completed, //!< Everything downloaded
    };

    /*!
     * @brief Report scraping stage completion
     * @param game Target game
     * @param stage Last stage completed
     */
    virtual void ScrapingStageCompleted(FileData* game, Stage stage, MetadataType changes) = 0;
};