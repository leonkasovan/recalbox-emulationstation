//
// Created by bkg2k on 17/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <scraping/scrapers/screenscraper/IEndPointProvider.h>
#include <utils/network/DnsClient.h>
#include <utils/os/system/Mutex.h>

class RecalboxEndPoints : public IEndPointProvider
{
  public:
    //! Constructor
    RecalboxEndPoints();

    /*!
     * @brief Try to get durect user instead of URL for the request
     * @return Direct user object or nullptr
     */
    const ScreenScraperUser* GetDirectUserObject() const override;

    /*!
     * @brief Get base url for relative media
     * @return Base url
     */
    String GetUrlBase() override;

    /*!
     * @brief Scraper call this when an unexpected error occured in HTTP requests (timeount, 500, ...)
     */
    void NotifyError() override;

    /*!
     * @brief Provide a user info Url built from the given credentials
     * @param login Login
     * @param password Password
     * @return User info Url
     */
    String GetUserInfoUrl(const String& login, const String& password) override;

    /*!
     * @brief Provide a game info Url built from the given parameters
     * @param login Login
     * @param password Password
     * @param system System ID
     * @param path Rom path
     * @param crc32 Rom CRC32
     * @param md5 Rom MD5
     * @param size Rom file size
     * @return Game info Url
     */
    String GetGameInfoUrl(const String& login, const String& password, const FileData& game,
                               const String& crc32, const String& md5, long long int size) override
    {
      (void)login;
      (void)password;
      (void)game;
      (void)crc32;
      (void)md5;
      (void)size;
      return String();
    }

    /*!
     * @brief Provide a game info Url built from the given parameters
     * Call it only if RequireSeparateRequests is true and a valid MD5 is provided
     * @param login Login
     * @param password Password
     * @param system System ID
     * @param path Rom path
     * @param crc32 Rom CRC32
     * @param md5 Rom MD5
     * @param size Rom file size
     * @return Game info Url
     */
    String GetGameInfoUrlByMD5(const String& login, const String& password, const FileData& game, const String& md5, long long size) override;

    /*!
     * @brief Provide a game info Url built from the given parameters
     * Call it only if RequireSeparateRequests is true and no MD5 is provided
     * @param login Login
     * @param password Password
     * @param system System ID
     * @param path Rom path
     * @param size Rom file size
     * @return Game info Url
     */
    String GetGameInfoUrlByName(const String& login, const String& password, const FileData& game, const String& md5, long long size) override;

    /*!
     * @brief Add (or not) query parameters to media request
     * @param url Url to decorate
     */
    void AddQueryParametersToMediaRequest(const FileData* game, long long size, String& url) override;

    /*!
     * @brief Recalbox do not use Basic Auth
     * @return False
     */
    bool RequireBasicAuth() override { return false; }

    /*!
     * @brief Recalbox uses Bearer
     * @return True
     */
    bool RequireBearer() override { return true; }

    /*!
     * @brief Tell the scraper, it needs to use separate requests whether MD5 hashes are available or not
     * @return
     */
    bool RequireSeparateRequests() override { return true; }

    /*!
     * @brief Provide screenscraper Web URL
     * @return Screenscraper Web url
     */
    String GetProviderWebURL() override { return "https://www.recalbox.com"; };

  private:
    //! Root domain name
    //static constexpr const char* sRootDomainName = "https://scraper-rocketeer.recalbox.com";
    static constexpr const char* sRootDomainName = "scrapers.recalbox.com";

    //! UUID
    String mUUID;
    //! Board
    String mBoard;
    //! Version
    String mVersion;

    //! Dns client
    DnsClient mDns;
    //! Mutex to protect error notification
    Mutex mErrorLocker;

    //! Target servers
    String::List mServers;
    //! Server index
    int mServerIndex;

    //! Total errors
    int mErrors;

    /*!
     * @brief Build standard query string
     * @param game Game to fetch data from
     * @return Query string
     */
    String BuildQueryString(const FileData* game, long long size);
};



