//
// Created by bkg2k on 17/04/23.
//
#pragma once


#include "SystemData.h"
#include "BaseSystemDownloader.h"

class DownloaderManager
{
  public:
    /*!
     * @brief Constructor
     * @param system System from whitch to get a downloader, if available
     */
    DownloaderManager();

    ~DownloaderManager();

    static bool HasDownloader(const SystemData& system);

    BaseSystemDownloader* CreateOrGetDownloader(SystemData& system, IGuiDownloaderUpdater& updater);

  private:
    //! Hold downloader per system
    HashMap<const SystemData*, BaseSystemDownloader*> mDownloaders;
};
