//
// Created by gugue_u on 03/01/2021.
//

#include <guis/menus/GuiMenuScraper.h>
#include <utils/locale/LocaleHelper.h>
#include <guis/MenuMessages.h>
#include <scraping/ScraperFactory.h>
#include <scraping/ScraperTools.h>
#include <guis/GuiScraperRun.h>
#include <guis/GuiMsgBox.h>
#include "GuiMenuNetwork.h"
#include "GuiMenuScreenScraperOptions.h"
#include <emulators/run/GameRunner.h>
#include <patreon/PatronInfo.h>

GuiMenuScraper::GuiMenuScraper(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("SCRAPER"), this),
  mSystemManager(systemManager)
{
  mScrapers = AddList<ScraperType>(_("SCRAPE FROM"), (int)Components::Scraper, this, GetScrapersEntries(), _(MENUMESSAGE_SCRAPER_FROM_HELP_MSG));

  // if (PatronInfo::Instance().IsPatron())
    AddSwitch(_("AUTOMATIC SCRAPING"), RecalboxConf::Instance().GetScraperAuto(), (int)Components::ScraperAuto, this);

  AddSwitch(_("LOCAL SCRAPING"), RecalboxConf::Instance().GetScraperLocal(), (int)Components::ScraperLocal, this);

  AddSubMenu(_("SCRAPER OPTIONS"), (int)Components::ScraperOptions);

  AddList<ScraperNameOptions>(_("GET GAME NAME FROM"), (int)Components::ScrapeNameFrom, this,
                              GetNameOptionsEntries(), _(MENUMESSAGE_SCRAPER_GET_NAME_FROM_HELP_MSG));

  mScrapingMethod = AddList<ScrapingMethod>(_("FILTER"), (int)Components::ScrapingMethod, nullptr, GetScrapingMethods(), "");

  mSystems = AddMultiList<SystemData*>(_("SYSTEMS"), (int)Components::Systems, nullptr, GetSystemsEntries(), "");

  // Buttons
  mMenu.addButton(_("SCRAPE NOW"), _("SCRAPE NOW"), [this] { start(); });
}

std::vector<GuiMenuBase::ListEntry<ScraperType>> GuiMenuScraper::GetScrapersEntries()
{
  std::vector<ListEntry<ScraperType>> list;
  for(const auto& kv : ScraperFactory::GetScraperList())
    list.push_back({ kv.second, kv.first, kv.first == RecalboxConf::Instance().GetScraperSource() });
  return list;
}

std::vector<GuiMenuBase::ListEntry<ScraperNameOptions>> GuiMenuScraper::GetNameOptionsEntries()
{
  ScraperNameOptions nameOption = RecalboxConf::Instance().GetScraperNameOptions();
  std::vector<ListEntry<ScraperNameOptions>> list;

  list.push_back({ _("Scraper results"), ScraperNameOptions::GetFromScraper, nameOption == ScraperNameOptions::GetFromScraper });
  list.push_back({ _("Raw filename"), ScraperNameOptions::GetFromFilename, nameOption == ScraperNameOptions::GetFromFilename });
  list.push_back({ _("Undecorated filename"), ScraperNameOptions::GetFromFilenameUndecorated, nameOption == ScraperNameOptions::GetFromFilenameUndecorated });

  return list;
}

std::vector<GuiMenuBase::ListEntry<ScrapingMethod>> GuiMenuScraper::GetScrapingMethods()
{
  std::vector<ListEntry<ScrapingMethod>> list;
  list.push_back({ _("All Games"), ScrapingMethod::All, false });
  list.push_back({ _("Only missing image"), ScrapingMethod::AllIfNoithingExists, true });
  return list;
}

std::vector<GuiMenuBase::ListEntry<SystemData*>> GuiMenuScraper::GetSystemsEntries()
{
  std::vector<ListEntry<SystemData*>> list;
  for(SystemData* system : mSystemManager.VisibleSystemList())
  {
    if (!system->IsVirtual() || system->IsFavorite() || system->IsPorts()) // Allow scraping favorites, but not virtual systems
      if (system->HasScrapableGame())
        list.push_back({ system->FullName(), system, false });
  }
  return list;
}

void GuiMenuScraper::OptionListComponentChanged(int id, int index, const ScraperType& value)
{
  (void)index;
  if ((Components)id == Components::Scraper)
    RecalboxConf::Instance().SetScraperSource(value).Save();
}

void GuiMenuScraper::OptionListComponentChanged(int id, int index, const ScraperNameOptions & value)
{
  (void)index;
  if ((Components)id == Components::ScrapeNameFrom)
    RecalboxConf::Instance().SetScraperNameOptions(value).Save();
}

void GuiMenuScraper::SubMenuSelected(int id)
{
  if ((Components)id == Components::ScraperOptions)
    switch (mScrapers->getSelected())
    {
      case ScraperType::TheGameDB:
      case ScraperType::Recalbox:
      case ScraperType::ScreenScraper:
          mWindow.pushGui(new GuiMenuScreenScraperOptions(mWindow, mSystemManager, RecalboxConf::Instance().GetScraperSource())); break;
    }
}

void GuiMenuScraper::start()
{
  if (mSystems->getSelectedObjects().empty())
  {
    String text = _("Please select one or more systems to scrape!");
    GuiMsgBox* msgBox = new GuiMsgBox(mWindow, text, _("OK"), nullptr);
    mWindow.pushGui(msgBox);
  }
  else
    GuiScraperRun::CreateOrShow(mWindow, mSystemManager, mSystems->getSelectedObjectsAsArray(), mScrapingMethod->getSelected(), &GameRunner::Instance(), Renderer::Instance().DisplayHeightAsInt() <=576);
}

const char *local_scrape_script = 
"directory=\"/recalbox/share/screenshots\"\n\
rom_fullpath=$6\n\
rom_extension=\"${rom_fullpath##*.}\"\n\
rom_filename=$(basename \"$rom_fullpath\")\n\
rom_title=$(basename \"$rom_fullpath\" \".$rom_extension\")\n\
rom_directory=$(dirname \"$rom_fullpath\")\n\
\n\
for file in \"$directory\"/*; do\n\
	if [[ -f \"$file\" ]]; then # Check if it's a regular file\n\
		if [[ $file == *\"$rom_title\"* ]]; then # Compare the filename with the specific string\n\
			source=\"$file\"\n\
			target=\"$rom_directory/media/images/$rom_title.png\"\n\
		fi\n\
	fi\n\
done\n\
mkdir -p \"$rom_directory/media/images\"\n\
cp \"$source\" \"$target\"\n";

void GuiMenuScraper::SwitchComponentChanged(int id, bool status)
{
  if ((Components)id == Components::ScraperAuto)
    RecalboxConf::Instance().SetScraperAuto(status).Save();
  else if ((Components)id == Components::ScraperLocal){
    RecalboxConf::Instance().SetScraperLocal(status).Save();
    if (status){  // ScraperLocal is true then create a script for event "EndGame"
      FILE *f;

      f = fopen("/recalbox/share/userscripts/scrape-screenshot[EndGame].sh","w");
      if (f){
        fputs(local_scrape_script, f);
        fclose(f);
      }
    }else{ // ScraperLocal is false then delete the script for event "EndGame"
      Path script("/recalbox/share/userscripts/scrape-screenshot[EndGame].sh");
      script.Delete();
    }
  }
    
}
