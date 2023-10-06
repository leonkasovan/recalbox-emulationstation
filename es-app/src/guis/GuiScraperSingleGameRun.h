#pragma once

#include <scraping/scrapers/IScraperEngine.h>
#include <components/ButtonComponent.h>
#include "guis/Gui.h"
#include "components/ScraperSearchComponent.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"

class GuiScraperSingleGameRun : public Gui, public INotifyScrapeResult
{
  public:
    class IScrapingComplete
    {
      public:
        //! Default destructor
        virtual ~IScrapingComplete() = default;

        /*!
         * @brief Notifification of a game scraping end
         * @param game Scrapped games
         * @param changedMetadata Changed metadata
         */
        virtual void ScrapingComplete(FileData& game, MetadataType changedMetadata) = 0;
    };

  private:
    //! SystemManager reference
    SystemManager& mSystemManager;

    //! Scraper interface
    IScraperEngine* mScraper;

    //! Target game
    FileData& mGame;

    //! Notification interface
    IScrapingComplete* mNotifier;

    /*
     * Components
     */

    ComponentGrid mGrid;
    NinePatchComponent mBox;

    std::shared_ptr<TextComponent> mGameName;
    std::shared_ptr<TextComponent> mSystemName;
    std::shared_ptr<ScraperSearchComponent> mSearch;
    std::shared_ptr<ButtonComponent> mButton;
    std::shared_ptr<ComponentGrid> mButtonGrid;

  public:
    explicit GuiScraperSingleGameRun(WindowManager&window, SystemManager& systemManager, FileData& game, IScrapingComplete* notifier);

    void onSizeChanged() override;

    bool ProcessInput(const InputCompactEvent& event) override;
    bool getHelpPrompts(Help& help) override;

    /*
     * INotifyScrapeResult implementation
     */

    /*!
     * @brief Notify a game has been scraped
     * @param index Game index
     * @param total Total game to scrape
     * @param result Result object
     */
    void GameResult(int index, int total, FileData* result, MetadataType changedMetadata) override;

    /*!
     * @brief Scraper site quota reached. Scraping is being aborted immediately.
     */
    void ScrapingComplete(ScrapeResult reason, MetadataType changedMetadata) override;
};
