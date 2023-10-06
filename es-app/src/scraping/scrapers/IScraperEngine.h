//
// Created by bkg2k on 04/12/2019.
//
#pragma once

#include <utils/cplusplus/INoCopy.h>
#include <systems/SystemManager.h>
#include <scraping/INotifyScrapeResult.h>
#include <scraping/scrapers/ScrapingMethod.h>

class IScraperEngine : private INoCopy
{
  public:
    /*!
     * @brief Called from the factory to reset the current instance to its initial state
     * Implementation is responsible for reseting all its internal structures as if it were just created.
     */
    virtual void Initialize() = 0;

    /*!
     * @brief Run the scraper using the given methods, on the given system list and report progress using notifyTarget
     * @param method Scraping method
     * @param systemList System list
     * @param notifyTarget Interface for reporting scraping progression
     * @return True if everything has been successful. False if cancelled, quota reached or fatal error occurred
     */
    virtual bool RunOn(ScrapingMethod method, const SystemManager::List& systemList,
                       INotifyScrapeResult* notifyTarget, long long diskMinimumFree) = 0;

    /*!
     * @brief Run the scraper using the given methods, on the given single game and report progress using notifyTarget
     * @param method Scraping method
     * @param singleGame Single game to scrape
     * @param notifyTarget Interface for reporting scraping progression
     * @return True if everything has been successful. False if cancelled, quota reached or fatal error occurred
     */
    virtual bool RunOn(ScrapingMethod method, FileData& singleGame,
                       INotifyScrapeResult* notifyTarget, long long diskMinimumFree) = 0;

    /*!
     * @brief Request the engine to abort as soon as possible!
     * @param waitforcompletion If true, wait for completion before exit
     * @return True
     */
    virtual bool Abort(bool waitforcompletion) = 0;

    /*!
     * @brief Check if the engine is running, allowing UI to know when the engine actually stops after an abort request
     * @return True if the engine is running
     */
    [[nodiscard]] virtual bool IsRunning() const = 0;

    //! Get total to scrape
    [[nodiscard]] virtual int ScrapesTotal() const = 0;

    //! Get processed items
    [[nodiscard]] virtual int ScrapesProcessed() const = 0;

    //! Get pending items (still not scraped)
    [[nodiscard]] virtual int ScrapesStillPending() const = 0;

    //! Get successfully scraped games
    [[nodiscard]] virtual int ScrapesSuccessful() const = 0;

    //! Get unsuccessfully scraped games
    [[nodiscard]] virtual int ScrapesNotFound() const = 0;

    //! Get failed scrapes
    [[nodiscard]] virtual int ScrapesErrors() const = 0;

    //! Stats Text infos
    [[nodiscard]] virtual int StatsTextInfo() const = 0;

    //! Stats images downloaded
    [[nodiscard]] virtual int StatsImages() const = 0;

    //! Stats videos downloaded
    [[nodiscard]] virtual int StatsVideos() const = 0;

    //! Stats videos downloaded
    [[nodiscard]] virtual long long StatsMediaSize() const = 0;

    //! Get Scraper message
    virtual String ScraperDatabaseMessage() = 0;

    /*!
     * @brief Virtual destructor
     */
    virtual ~IScraperEngine() = default;

    /*!
     * @brief Stop notifications (Nullify INotifyScrapeResult)
     */
    virtual void StopNotifications() = 0;
};
