//
// Created by bkg2k on 04/12/2019.
//
#pragma once

#include <scraping/scrapers/IScraperEngine.h>
#include <scraping/scrapers/IScraperEngineFreezer.h>

class TheGameDBEngine : public IScraperEngine
{
  private:
    /*
     * IScraperEngine implemntation
     */

    /*!
     * @brief Called from the factory to reset the current instance to its initial state
     * Implementation is responsible for reseting all its internal structures as if it were just created.
     */
    void Initialize() override;

    /*!
     * @brief Run the scraper using the given methods, on the given system list and report progress using notifyTarget
     * @param method Scraping method
     * @param systemList System list
     * @param notifyTarget Interface for reporting scraping progression
     * @return True if everything has been successful. False if cancelled, quota reached or fatal error occurred
     */
    bool RunOn(ScrapingMethod method, const SystemManager::List& systemList, INotifyScrapeResult* notifyTarget,
               long long diskMinimumFree) override;

    /*!
     * @brief Run the scraper using the given methods, on the given single game and report progress using notifyTarget
     * @param method Scraping method
     * @param singleGame Single game to scrape
     * @param notifyTarget Interface for reporting scraping progression
     * @return True if everything has been successful. False if cancelled, quota reached or fatal error occurred
     */
    bool RunOn(ScrapingMethod method, FileData& singleGame,
               INotifyScrapeResult* notifyTarget, long long diskMinimumFree) override;

    //! Get total to scrape (pendings + processed)
    [[nodiscard]] int ScrapesTotal() const override { return 0; }

    //! Get processed items
    [[nodiscard]] int ScrapesProcessed() const override { return 0; }

    //! Get pending items (still not scraped)
    [[nodiscard]] int ScrapesStillPending() const override { return 0; }

    //! Get successfully scraped games
    [[nodiscard]] int ScrapesSuccessful() const override { return 0; }

    //! Get unsuccessfully scraped games
    [[nodiscard]] int ScrapesNotFound() const override { return 0; }

    //! Get failes scrapes
    [[nodiscard]] int ScrapesErrors() const override { return 0; }

    //! Stats Text infos
    [[nodiscard]] int StatsTextInfo() const override { return 0; }

    //! Stats images downloaded
    [[nodiscard]] int StatsImages() const override { return 0; }

    //! Stats videos downloaded
    [[nodiscard]] int StatsVideos() const override { return 0; }

    //! Stats videos downloaded
    [[nodiscard]] long long StatsMediaSize() const override { return 0; }

    //! Get Scraper message
    String ScraperDatabaseMessage() override { return String(); };

    /*!
     * @brief Abort the current engine
     * @param waitforcompletion If true, wait for completion before exit
     * @return True
     */
    bool Abort(bool waitforcompletion) override;

    /*!
     * @brief Check if the engine is running, allowing UI to know when the engine actually stops after an abort request
     * @return True if the engine is running
     */
    [[nodiscard]] bool IsRunning() const override;

    /*!
     * @brief Stop notifications (Nullify INotifyScrapeResult)
     */
    void StopNotifications() override {}

  public:
    explicit TheGameDBEngine(IScraperEngineFreezer* freezer) { (void)freezer; };
};
