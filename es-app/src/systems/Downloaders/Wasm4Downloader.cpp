//
// Created by bkg2k on 17/04/23.
//

#include "Wasm4Downloader.h"
#include "systems/SystemManager.h"
#include "utils/Zip.h"
#include "utils/Files.h"
#include "utils/locale/LocaleHelper.h"

Wasm4Downloader::Wasm4Downloader(SystemData& wasm4, IGuiDownloaderUpdater& updater)
  : BaseSystemDownloader(updater)
  , mSender(*this)
  , mSystem(wasm4)
  , mTotalSize(0)
  , mCurrentSize(0)
  , mGames(0)
{
}

void Wasm4Downloader::DownloadAndInstall()
{
  mSender.Send(Wasm4DownloadingGameState::Start);
  usleep(20000); // Let display refreshing
  { LOG(LogDebug) << "[Wasm4Downloader] Download files for " << mSystem.FullName(); }

  // Seek for the right folder
  const RootFolderData& roots = mSystem.MasterRoot();
  RootFolderData* targetRoot = nullptr;
  Path output;
  for(RootFolderData* root : roots.SubRoots())
    if (!root->ReadOnly() && root->RomPath().StartWidth("/recalbox/share/roms/"))
    {
      output = root->RomPath();
      targetRoot = root;
    }
  if (output.IsEmpty()) { mSender.Send(Wasm4DownloadingGameState::WriteOnlyShare); return; }

  // Get destination filename
  Path destination("/recalbox/share/system/download.tmp");
  { LOG(LogDebug) << "[Wasm4Downloader] Target path " << destination.ToString(); }

  // Source URL
  String source(sRepoURL);

  // Download
  if (mStopAsap) return;
  (void)destination.Delete();
  mTimeReference = DateTime();
  if (!mRequest.Execute(source, destination, this)) { mSender.Send(Wasm4DownloadingGameState::DownloadError); return; }

  // Extract
  { LOG(LogDebug) << "[Wasm4Downloader] Extracting games"; }
  Zip zip(destination);
  // Mini storage of the wasm game, its md file and its png file
  struct wasmProps { int wasm = -1, md = -1, png = -1; };
  HashMap<Path, wasmProps> wasms;
  // Fill in storage
  mTotalSize = zip.Count();
  mCurrentSize = 0;
  for(int i = (int)mTotalSize; --i >= 0;)
  {
    if (mStopAsap) return;
    Path relativePath(zip.FileName(i));
    Path destinationPath = output / relativePath.Filename();
    if (relativePath.Extension() == ".wasm")
    {
      Files::SaveFile(destinationPath, zip.Content(i));
      mGames++;
      wasms[destinationPath].wasm = i;
    }
    if (relativePath.Extension() == ".md"  ) wasms[destinationPath.ChangeExtension(".wasm")].md = i;
    if (relativePath.Extension() == ".png" ) wasms[destinationPath.ChangeExtension(".wasm")].png = i;
    mCurrentSize++;
    mSender.Send(Wasm4DownloadingGameState::Extracting);
    usleep(20000); // Let display refreshing
  }

  // Populate
  { LOG(LogDebug) << "[Wasm4Downloader] Populate gamelist"; }
  mTotalSize = wasms.size();
  mCurrentSize = 0;
  mSender.Send(Wasm4DownloadingGameState::UpdatingMetadata);
  FileData::StringMap doppleGanger;
  if (mStopAsap) return;
  targetRoot->BuildDoppelgangerMap(doppleGanger, true);
  if (mStopAsap) return;
  targetRoot->PopulateRecursiveFolder(*targetRoot, ".wasm", "", doppleGanger);

  // Update metadata
  { LOG(LogDebug) << "[Wasm4Downloader] Update metadata"; }
  for(const auto& kv : wasms)
  {
    if (mStopAsap) return;
    const wasmProps& props = kv.second;
    FileData* game = targetRoot->LookupGameByFilePath(kv.first.ToString());
    if (game != nullptr)
    {
      // Name
      if (game->Metadata().Name().empty()) game->Metadata().SetName(kv.first.FilenameWithoutExtension());
      // Image
      if (game->Metadata().Image().IsEmpty() && props.png >= 0)
      {
        Path imageOutput = (output / "media" / "images" / kv.first.Filename()).ChangeExtension(".png");
        (void)imageOutput.Directory().CreatePath();
        Files::SaveFile(imageOutput, zip.Content(props.png));
        game->Metadata().SetImagePath(imageOutput);
      }
      // Author / Description / Data
      if (props.md >= 0)
      {
        String desc;
        String author;
        DateTime date(0LL);
        int breakers = 0;
        for(String& line : String(zip.Content(props.md)).Split('\n'))
          if (!line.Trim("\t\r ").empty())
          {
            if (line.StartsWith("---")) { breakers++; continue; }
            if (breakers == 1)
            {
              if (line.StartsWith("author: "))
                author = line.Remove(LEGACY_STRING("author: "));
              if (line.StartsWith("date: "))
                DateTime::ParseFromString("%yyyy-%MM-%dd", line.Remove("date: "), date);
            }
            if (breakers == 2)
              desc.Append(line.Replace('(', ' ').Remove(").").Remove(')').Remove('[').Remove("# ").Replace(']', ':').Append(String::CRLF));
          }
        if (game->Metadata().Description().empty() && !desc.empty())
          game->Metadata().SetDescription(desc);
        if (game->Metadata().Developer().empty() && !author.empty())
          game->Metadata().SetDeveloper(author);
        if (game->Metadata().ReleaseDate().Year() < 2000 && !date.IsZero())
          game->Metadata().SetReleaseDate(date);
      }
    }
    else { LOG(LogError) << "[Wasm4Downloader] Game " << kv.first.ToString() << " not found in gamelist!"; }
    mCurrentSize++;
    mSender.Send(Wasm4DownloadingGameState::UpdatingMetadata);
    usleep(20000); // Let display refreshing
  }

  // Delete temp file
  (void)destination.Delete();
}

void Wasm4Downloader::DownloadProgress(const HttpClient& http, long long int currentSize, long long int expectedSize)
{
  (void)http;
  // Store data and synchronize
  mTotalSize = expectedSize;
  mCurrentSize = currentSize;
  mSender.Send(Wasm4DownloadingGameState::Downloading);
}

void Wasm4Downloader::ReceiveSyncMessage(const Wasm4DownloadingGameState& code)
{
  switch(code)
  {
    case Wasm4DownloadingGameState::Start:
    {
      String title(_("DOWNLOADING GAMES FOR %s"));
      mUpdater.UpdateTitleText(title.Replace("%s", "WASM4"));
      mUpdater.UpdateMainText(_("Downloading WASM4 games from the official site. Please wait..."));
      break;
    }
    case Wasm4DownloadingGameState::Downloading:
    {
      // Load size into progress bar component
      mUpdater.UpdateProgressbar(mCurrentSize, mTotalSize);

      // Elapsed time
      if (mCurrentSize != 0 && mCurrentSize < mTotalSize)
      {
        TimeSpan elapsed = DateTime() - mTimeReference;
        TimeSpan eta((elapsed.TotalMilliseconds() * (mTotalSize - mCurrentSize)) / mCurrentSize);

        String text = _("Downloading... Estimated time: %s").Replace("%s", eta.ToTimeString());
        mUpdater.UpdateETAText(text);
      }
      break;
    }
    case Wasm4DownloadingGameState::Extracting:
    {
      // Load size into progress bar component
      mUpdater.UpdateProgressbar(mCurrentSize, mTotalSize);

      String text = _("Extracting... found %s games").Replace("%s", String(mGames));
      mUpdater.UpdateETAText(text);
      break;
    }
    case Wasm4DownloadingGameState::UpdatingMetadata:
    {
      // Load size into progress bar component
      mUpdater.UpdateProgressbar(mCurrentSize, mTotalSize);

      String text = _("Updating metadata...").Replace("%s", String(mCurrentSize));
      if (mCurrentSize == 0) text = _("Refreshing gamelist...");
      mUpdater.UpdateETAText(text);
      break;
    }
    case Wasm4DownloadingGameState::WriteOnlyShare:
    {
      mUpdater.UpdateETAText("Can't write games to share!");
      break;
    }
    case Wasm4DownloadingGameState::DownloadError:
    {
      mUpdater.UpdateETAText("Error downloading games! Retry later.");
      break;
    }
  }
}

void Wasm4Downloader::Completed(bool stopped)
{
  mUpdater.DownloadComplete(mSystem, stopped);
}

