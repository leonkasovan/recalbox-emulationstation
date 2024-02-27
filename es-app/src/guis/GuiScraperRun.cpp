#include <systems/SystemManager.h>
#include <scraping/ScraperFactory.h>
#include <MainRunner.h>
#include <recalbox/RecalboxSystem.h>
#include "guis/GuiScraperRun.h"

#include "components/ScraperSearchComponent.h"
#include "components/MenuComponent.h" // for makeButtonGrid
#include "guis/GuiMsgBox.h"
#include "MenuMessages.h"
#include "recalbox/RecalboxStorageWatcher.h"

GuiScraperRun* GuiScraperRun::sInstance = nullptr;

void GuiScraperRun::CreateOrShow(WindowManager& window, SystemManager& systemManager, const SystemManager::List& systems, ScrapingMethod method, IScraperEngineFreezer* freezer, bool lowResolution)
{
  if (IsRunning())
    Show(window);
  else
    window.pushGui(sInstance = new GuiScraperRun(window, systemManager, systems, method, freezer, lowResolution));
}

void GuiScraperRun::Show(WindowManager& window)
{
  if (sInstance != nullptr)
  {
    window.RemoveGui(sInstance);
    window.pushGui(sInstance);
    window.InfoPopupRemove(&sInstance->mPopup);
  }
}

void GuiScraperRun::Hide(WindowManager& window)
{
  if (sInstance != nullptr)
  {
    window.RemoveGui(sInstance);
    if(sInstance->mLowResolution)
      window.InfoPopupAdd(new GuiInfoPopup(window, _("Scrap running in background.\nReturn to the scrap menu to get the progression."), 8, PopupType::Scraper));
    else
      window.InfoPopupAdd(&sInstance->mPopup, true);
    window.CloseAll();
  }
}

void GuiScraperRun::Terminate()
{
  if (sInstance != nullptr)
    sInstance->Close();
  sInstance = nullptr;
}

GuiScraperRun::GuiScraperRun(WindowManager& window, SystemManager& systemManager, const SystemManager::List& systems, ScrapingMethod method, IScraperEngineFreezer* freezer, bool lowResolution)
  :	Gui(window)
  , mSystemManager(systemManager)
  , mResult(ScrapeResult::NotScraped)
  , mSearchQueue(systems)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(1, 8))
  , mLowResolution(lowResolution)
  , mPopup(window)
{
	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
	
	mBackground.setImagePath(menuTheme->menuBackground.path);
	mBackground.setCenterColor(menuTheme->menuBackground.color);
	mBackground.setEdgeColor(menuTheme->menuBackground.color);
	
	assert(!mSearchQueue.Empty());

	addChild(&mBackground);
	addChild(&mGrid);

	mIsProcessing = true;

	// set up grid
	mTitle = std::make_shared<TextComponent>(mWindow, _("SCRAPING IN PROGRESS"), lowResolution ? menuTheme->menuText.font : menuTheme->menuTitle.font, menuTheme->menuTitle.color, TextAlignment::Center);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

  mSystem = std::make_shared<TextComponent>(mWindow, _("SYSTEM"), menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Center);
	mGrid.setEntry(mSystem, Vector2i(0, 1), false, true);

	mSubtitle = std::make_shared<TextComponent>(mWindow, "", menuTheme->menuFooter.font, menuTheme->menuFooter.color, TextAlignment::Center);
	mGrid.setEntry(mSubtitle, Vector2i(0, 2), false, true);

	mSearchComp = std::make_shared<ScraperSearchComponent>(mWindow,lowResolution);
	mGrid.setEntry(mSearchComp, Vector2i(0, 3), false, true);

  // Progress bar
  mProgressGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(3, 3));
  mBar = std::make_shared<ProgressBarComponent>(mWindow, 1);
  mProgressGrid->setEntry(mBar, Vector2i(1, 1), false, true);
  mGrid.setEntry(mProgressGrid, Vector2i(0,4), false, true);
  mProgressGrid->setColWidthPerc(0, 0.02f);
  mProgressGrid->setColWidthPerc(1, 0.96f);
  mProgressGrid->setColWidthPerc(2, 0.02f);
  mProgressGrid->setRowHeightPerc(0, 0.2f);
  mProgressGrid->setRowHeightPerc(1, 0.6f);
  mProgressGrid->setRowHeightPerc(2, 0.2f);

  mTiming = std::make_shared<TextComponent>(mWindow, "", menuTheme->menuFooter.font, menuTheme->menuText.color, TextAlignment::Center);
  mGrid.setEntry(mTiming, Vector2i(0, 5), false, true);


  mDatabaseMessage = std::make_shared<TextComponent>(mWindow, "", menuTheme->menuFooter.font, menuTheme->menuFooter.color, TextAlignment::Center);
  if(!lowResolution)
    mGrid.setEntry(mDatabaseMessage, Vector2i(0, 6), false, true);

  std::vector<std::shared_ptr<ButtonComponent>> buttons
  {
    mButton = std::make_shared<ButtonComponent>(mWindow, _("STOP"), _("stop (progress saved)"), std::bind(&GuiScraperRun::finish, this)),
    mRunInBgButton = std::make_shared<ButtonComponent>(mWindow, _("RUN IN BACKGROUND"), _("RUN IN BACKGROUND"), [this] { GuiScraperRun::Hide(mWindow); })
  };
	mButtonGrid = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 7), true, false);

	setSize(Renderer::Instance().DisplayWidthAsFloat() * 0.95f, Renderer::Instance().DisplayHeightAsFloat() * (lowResolution ? 0.78f : 0.849f));
	setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

	// Final report component
  mFinalReport = std::make_shared<TextComponent>(mWindow, _("subtitle text"), menuTheme->menuFooter.font, menuTheme->menuText.color, TextAlignment::Center);

  // Scripts
  NotificationManager::Instance().Notify(Notification::ScrapStart);

  // Create scraper and run!
	mScraper = ScraperFactory::Instance().GetScraper(RecalboxConf::Instance().GetScraperSource(), freezer);
	mScraper->RunOn(method, systems, this, (long long)RecalboxSystem::GetMinimumFreeSpaceOnSharePartition());
}

void GuiScraperRun::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mGrid.setRowHeightPerc(0, mTitle->getFont()->getLetterHeight() * 2.0f / mSize.y(), false);
	mGrid.setRowHeightPerc(1, (mSystem->getFont()->getLetterHeight() + 2.0f) / mSize.y(), false);
	mGrid.setRowHeightPerc(2, mSubtitle->getFont()->getHeight() * 1.75f / mSize.y(), false);
  mGrid.setRowHeightPerc(4, 0.04f, false);
  mGrid.setRowHeightPerc(5, mTiming->getFont()->getHeight() * 1.5f / mSize.y(), false);
  mGrid.setRowHeightPerc(6, mDatabaseMessage->getFont()->getHeight() * 1.5f / mSize.y(), false);
  mGrid.setRowHeightPerc(7, mButtonGrid->getSize().y() / mSize.y(), false);
	mGrid.setSize(mSize);
}

void GuiScraperRun::finish()
{
  mScraper->Abort(true);
  Terminate();
  for(const auto& systemData : mSearchQueue)
    systemData->UpdateGamelistXml();
  mWindow.CloseAll();
  /*switch(mResult)
  {
    case ScrapeResult::Ok: MainRunner::RequestQuit(MainRunner::ExitState::Relaunch, true); break;
    case ScrapeResult::NotScraped:
    case ScrapeResult::NotFound:
    case ScrapeResult::QuotaReached:
    case ScrapeResult::DiskFull:
    case ScrapeResult::FatalError:
    default: break;
  }*/
  ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
	mIsProcessing = false;
}

void GuiScraperRun::GameResult(int index, int total, FileData* result, MetadataType changedMetadata)
{
  (void)changedMetadata; // #TODO: see later if it's worth the try to refresh systems game per game
  switch(RecalboxConf::Instance().GetScraperNameOptions())
  {
    case ScraperNameOptions::GetFromScraper: break;
    case ScraperNameOptions::GetFromFilename:
    {
      result->Metadata().SetName(result->RomPath().FilenameWithoutExtension());
      break;
    }
    case ScraperNameOptions::GetFromFilenameUndecorated:
    {
      ScraperFactory::ExtractFileNameUndecorated(*result);
      break;
    }
  }

  // Update popup
  mPopup.SetScrapedGame(*result, index, total);

  // update title
  mSystem->setText(result->System().FullName().ToUpperCaseUTF8());

  // update subtitle
  mSubtitle->setText(result->RomPath().Filename().ToUpperCase());

  mBar->setMaxValue(total);
  mBar->setCurrentValue(index);

  // Update game data
  mSearchComp->UpdateInfoPane(result);

  // Update timings
  TimeSpan elapsed = DateTime() - mStart;

  String status = (_F(_("GAME {0} OF {1}")) / index / total).ToString().Append(" - ");
  if(!mLowResolution)
    status.Append(_("ELAPSED TIME: "))
          .Append(elapsed.ToStringFormat("%H:%mm:%ss"))
          .Append(" - ");
  status.Append(_("ESTIMATED TIME: "));

  if ((mScraper->ScrapesProcessed() > 10 || mScraper->ScrapesTotal() <= 10) && elapsed.TotalSeconds() > 10)
  {
    long long millisecondPerGame = elapsed.TotalMilliseconds() / mScraper->ScrapesProcessed();
    TimeSpan estimated(millisecondPerGame * mScraper->ScrapesStillPending());
    if (mScraper->ScrapesProcessed() < mScraper->ScrapesTotal())
      status.Append(estimated.ToStringFormat("%H:%mm:%ss"));
    else
      status.Append(_("COMPLETE!"));
  }
  else status.Append("---");

  if (mScraper->ScrapesProcessed() >= mScraper->ScrapesTotal() && mLowResolution)
    status = "";
  mTiming->setText(status);

  // Check free space
  RecalboxStorageWatcher::CheckStorageFreeSpace(mWindow, mSystemManager.GetMountMonitor(), result->RomPath());

  // Update database message
  mDatabaseMessage->setText(mScraper->ScraperDatabaseMessage());

  // Scripts
  NotificationManager::Instance().Notify(*result, Notification::ScrapGame);
}

void GuiScraperRun::ScrapingComplete(ScrapeResult reason, MetadataType changedMetadata)
{
  mGrid.removeEntry(mSearchComp);
  String finalReport;
  switch(mResult = reason)
  {
    case ScrapeResult::Ok:
    case ScrapeResult::NotScraped:
    case ScrapeResult::NotFound:
    {
      finalReport = _(MENUMESSAGE_SCRAPER_FINAL_POPUP);
      finalReport.Replace("{PROCESSED}", String(mScraper->ScrapesTotal()))
                 .Replace("{SUCCESS}", String(mScraper->ScrapesSuccessful()))
                 .Replace("{NOTFOUND}", String(mScraper->ScrapesNotFound()))
                 .Replace("{ERRORS}", String(mScraper->ScrapesErrors()))
                 .Replace("{TEXTINFO}", String(mScraper->StatsTextInfo()))
                 .Replace("{IMAGES}", String(mScraper->StatsImages()))
                 .Replace("{VIDEOS}", String(mScraper->StatsVideos()));
      long long size = mScraper->StatsMediaSize();
      String sizeText;
      if      (size >= (1 << 30)) sizeText = String((float)(size >> 20) / 1024.0f, 2).Append("GB");
      else if (size >= (1 << 20)) sizeText = String((float)(size >> 10) / 1024.0f, 2).Append("MB");
      else if (size >= (1 << 10)) sizeText = String((float)size / 1024.0f, 2).Append("KB");
      else                        sizeText = String((int)size).Append("B");
      finalReport = finalReport.Replace("{MEDIASIZE}", sizeText).ToUpperCaseUTF8();
      break;
    }
    case ScrapeResult::FatalError:
    {
      finalReport = _(MENUMESSAGE_SCRAPER_FINAL_FATAL);
      break;
    }
    case ScrapeResult::QuotaReached:
    {
      finalReport = _(MENUMESSAGE_SCRAPER_FINAL_QUOTA);
      break;
    }
    case ScrapeResult::DiskFull:
    {
      finalReport = _(MENUMESSAGE_SCRAPER_FINAL_DISKFULL);
      break;
    }
  }
  mFinalReport->setText(finalReport);
  mGrid.setEntry(mFinalReport, Vector2i(0, 3), false, true);

  // Update popup
  mPopup.ScrapingComplete(reason);

  // Refresh systems
  switch(reason)
  {
    case ScrapeResult::Ok:
    case ScrapeResult::NotScraped:
    case ScrapeResult::NotFound:
    {
      mSystemManager.UpdateSystemsOnGameChange(nullptr, changedMetadata, false);
      break;
    }
    case ScrapeResult::QuotaReached:
    case ScrapeResult::DiskFull:
    case ScrapeResult::FatalError:
    default: break;
  }


  // Update button?
  mButton->setText(_("CLOSE"), _("CLOSE"));
  mSearchComp->SetRunning(false);
  mGrid.removeEntry(mButtonGrid);
  mButtonGrid->removeEntry(mButton);
  mButtonGrid = makeButtonGrid(mWindow,  std::vector<std::shared_ptr<ButtonComponent>>
  {
    mButton
  });
  mGrid.setEntry(mButtonGrid, Vector2i(0, 7), true, false);

  // Scripts
  NotificationManager::Instance().Notify(Notification::ScrapStop, String(mScraper->ScrapesSuccessful()));

  // Hiden?
  if (mWindow.InfoPopupIsShown(&mPopup) || mLowResolution)
  {
    String text = mLowResolution ? _("Your scraping session completed!") : _("Your scraping session completed. Press OK to show the results.");
    GuiMsgBox* msgBox = new GuiMsgBox(mWindow, text, _("OK"), [this] { if (mLowResolution) Terminate(); else Show(mWindow); mButtonGrid->resetCursor(); });
    mWindow.pushGui(msgBox);
  }
}

