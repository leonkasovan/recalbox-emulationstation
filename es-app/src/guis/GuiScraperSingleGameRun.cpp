#include "guis/GuiScraperSingleGameRun.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "components/TextComponent.h"
#include "components/ButtonComponent.h"
#include "components/MenuComponent.h"
#include "utils/locale/LocaleHelper.h"
#include "themes/MenuThemeData.h"
#include "usernotifications/NotificationManager.h"
#include "recalbox/RecalboxStorageWatcher.h"
#include <scraping/ScraperFactory.h>
#include <recalbox/RecalboxSystem.h>
#include <scraping/ScraperTools.h>

GuiScraperSingleGameRun::GuiScraperSingleGameRun(WindowManager&window, SystemManager& systemManager, FileData& game, IScrapingComplete* notifier)
  : Gui(window)
  , mSystemManager(systemManager)
  , mScraper(ScraperFactory::Instance().GetScraper(RecalboxConf::Instance().GetScraperSource(), nullptr))
  , mGame(game)
  , mNotifier(notifier)
  , mGrid(window, Vector2i(1, 7))
  , mBox(window, Path(":/frame.png"))
{
	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
	
	mBox.setImagePath(menuTheme->menuBackground.path);
	mBox.setCenterColor(menuTheme->menuBackground.color);
	mBox.setEdgeColor(menuTheme->menuBackground.color);
	addChild(&mBox);
	addChild(&mGrid);

	// row 0 is a spacer

	mGameName = std::make_shared<TextComponent>(mWindow, game.RomPath().Filename().UpperCase(), menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Center);
	mGrid.setEntry(mGameName, Vector2i(0, 1), false, true);

	// row 2 is a spacer

	mSystemName = std::make_shared<TextComponent>(mWindow, game.System().FullName().ToUpperCase(), menuTheme->menuTextSmall.font, menuTheme->menuTextSmall.color, TextAlignment::Center);
	mGrid.setEntry(mSystemName, Vector2i(0, 3), false, true);

	// row 4 is a spacer

	// ScraperSearchComponent
	mSearch = std::make_shared<ScraperSearchComponent>(window, Renderer::Instance().DisplayHeightAsInt()<=576);
	mGrid.setEntry(mSearch, Vector2i(0, 5), false);

	// buttons
	mButton = std::make_shared<ButtonComponent>(mWindow, _("CANCEL"), _("CANCEL"), [this] { Close(); });
	std::vector< std::shared_ptr<ButtonComponent> > buttons;
	buttons.push_back(mButton);
	mButtonGrid = makeButtonGrid(mWindow, buttons);

	mGrid.setEntry(mButtonGrid, Vector2i(0, 6), true, false);

	setSize(Renderer::Instance().DisplayWidthAsFloat() * 0.95f, Renderer::Instance().DisplayHeightAsFloat() * 0.747f);
	setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

  // Run!
  mScraper->RunOn(ScrapingMethod::All, game, this, (long long)RecalboxSystem::GetMinimumFreeSpaceOnSharePartition());
}

void GuiScraperSingleGameRun::onSizeChanged()
{
	mBox.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mGrid.setRowHeightPerc(0, 0.04f, false);
	mGrid.setRowHeightPerc(1, mGameName->getFont()->getLetterHeight() / mSize.y(), false); // game name
	mGrid.setRowHeightPerc(2, 0.04f, false);
	mGrid.setRowHeightPerc(3, mSystemName->getFont()->getLetterHeight() / mSize.y(), false); // system name
	mGrid.setRowHeightPerc(4, 0.04f, false);
	mGrid.setRowHeightPerc(6, mButtonGrid->getSize().y() / mSize.y(), false); // buttons
	mGrid.setSize(mSize);
}

bool GuiScraperSingleGameRun::ProcessInput(const InputCompactEvent& event)
{
	if (event.CancelPressed())
	{
    mScraper->StopNotifications();
    Close();
		return true;
	}

	return Component::ProcessInput(event);
}

bool GuiScraperSingleGameRun::getHelpPrompts(Help& help)
{
	return mGrid.getHelpPrompts(help);
}


void GuiScraperSingleGameRun::GameResult(int index, int total, FileData* result, MetadataType changedMetadata)
{
  (void)index;
  (void)total;

  // #TODO: move into scraper!
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

  // Check free space
  RecalboxStorageWatcher::CheckStorageFreeSpace(mWindow, mSystemManager.GetMountMonitor(), result->RomPath());

  // Update game data
  mSearch->SetRunning(false);
  mSearch->UpdateInfoPane(&mGame);
  if (mNotifier != nullptr) mNotifier->ScrapingComplete(mGame, changedMetadata);
  mSystemManager.UpdateSystemsOnGameChange(result, changedMetadata, false);

  // Scripts
  NotificationManager::Instance().Notify(*result, Notification::ScrapGame);
}

void GuiScraperSingleGameRun::ScrapingComplete(ScrapeResult reason, MetadataType changedMetadata)
{
  (void)reason;
  (void)changedMetadata;
  // Update UI
  mButton->setText(_("CLOSE"), _("CLOSE"));
}