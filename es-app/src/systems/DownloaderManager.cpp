//
// Created by bkg2k on 17/04/23.
//

#include "DownloaderManager.h"
#include "systems/Downloaders/Wasm4Downloader.h"
#include "systems/Downloaders/GenericDownloader.h"
// #include "systems/Downloaders/FbneoDownloader.h"

DownloaderManager::DownloaderManager()
{
  LOG(LogInfo) << "[DownloadManager] Manager instance created";
}

DownloaderManager::~DownloaderManager()
{
  for (const auto& kv : mDownloaders)
    delete kv.second;
}

bool DownloaderManager::HasDownloader(const SystemData& system)
{
  return system.Descriptor().HasDownloader() || system.Name() == "fbneo";
}

BaseSystemDownloader* DownloaderManager::CreateOrGetDownloader(SystemData& system, IGuiDownloaderUpdater& updater)
{
  BaseSystemDownloader* newDownloader = nullptr;
  if (HasDownloader(system))
  {
    BaseSystemDownloader** downloader = mDownloaders.try_get(&system);
    if (downloader != nullptr) return *downloader;

    if (system.Name() == "wasm4") newDownloader = new Wasm4Downloader(system, updater);
    else newDownloader = new GenericDownloader(system, updater);
    mDownloaders[&system] = newDownloader;
  }
  return newDownloader;
}

BaseSystemDownloader* DownloaderManager::CreateOrGetDownloader(SystemData& system, String url, IGuiDownloaderUpdater& updater)
{
  BaseSystemDownloader* newDownloader = nullptr;
  if (HasDownloader(system))
  {
    BaseSystemDownloader** downloader = mDownloaders.try_get(&system);
    if (downloader != nullptr) return *downloader;

    // if (system.Name() == "fbneo") 
    //   newDownloader = new FbneoDownloader(system, url, updater);
    // else 
    newDownloader = new GenericDownloader(system, url, updater);
    mDownloaders[&system] = newDownloader;
  }
  return newDownloader;
}
