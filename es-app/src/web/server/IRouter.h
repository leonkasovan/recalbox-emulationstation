//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include <pistache/include/pistache/router.h>
#include <pistache/include/pistache/http.h>
#include <utils/Log.h>

using namespace Pistache;

class IRouter
{
  private:
    Rest::Router mRouter;

  protected:
    /*!
     * @brief Handle files
     * @param request Request object
     * @param response Response object
     */
    virtual void FileServer(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*
     * Routes
     */

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    virtual void Versions(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET architecture
     * @param request Request object
     * @param response Response object
     */
    virtual void Architecture(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET cpu information
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemInfo(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET storage information
     * @param request Request object
     * @param response Response object
     */
    virtual void StorageInfo(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET bios download
     * @param request Request object
     * @param response Response object
     */
    virtual void BiosDownload(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET bios upload
     * @param request Request object
     * @param response Response object
     */
    virtual void BiosUpload(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get array of all system's bios
     * @param request Request object
     * @param response Response object
     */
    virtual void BiosGetAll(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get bios for the given system
     * @param request Request object
     * @param response Response object
     */
    virtual void BiosGetSystem(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get all systems
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsGetAll(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get all active systems
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsGetActives(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system console svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsResourceGetConsole(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system controller svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsResourceGetController(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system controls svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsResourceGetControls(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system game support svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsResourceGetGame(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system logo svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemsResourceGetLogo(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void ConfigurationGet(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST set configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void ConfigurationSet(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle OPTIONS get configuration keys, type and validators
     * @param request Request object
     * @param response Response object
     */
    virtual void ConfigurationOptions(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle DELETE delete configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void ConfigurationDelete(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get system configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemConfigurationGet(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST set system configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemConfigurationSet(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle OPTIONS get system configuration keys, type and validators
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemConfigurationOptions(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle DELETE delete system configuration keys
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemConfigurationDelete(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle OPTIONS get media options
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaOptions(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET get media list
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaGetList(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle DELETE delete media
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaDelete(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST create screenshot
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaTakeScreenshot(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET to get game media
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaGet(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET to get screenshot media
     * @param request Request object
     * @param response Response object
     */
    virtual void MediaGetScreenshot(const Rest::Request& request, Http::ResponseWriter response) = 0;

  public:
    /*!
     * @brief Constructor. Set all routes
     */
    IRouter()
    {
      // Versions
      Rest::Routes::Get(mRouter, "/api/versions", Rest::Routes::bind(&IRouter::Versions, this));
      // Architecture
      Rest::Routes::Get(mRouter, "/api/architecture", Rest::Routes::bind(&IRouter::Architecture, this));
      // Monitoring
      Rest::Routes::Get(mRouter, "/api/monitoring/systeminfo", Rest::Routes::bind(&IRouter::SystemInfo, this));
      Rest::Routes::Get(mRouter, "/api/monitoring/storageinfo", Rest::Routes::bind(&IRouter::StorageInfo, this));
      // Bios
      Rest::Routes::Get(mRouter, "/api/bios", Rest::Routes::bind(&IRouter::BiosGetAll, this));
      Rest::Routes::Get(mRouter, "/api/bios/get/*", Rest::Routes::bind(&IRouter::BiosGetSystem, this));
      Rest::Routes::Get(mRouter, "/api/bios/download", Rest::Routes::bind(&IRouter::BiosDownload, this));
      Rest::Routes::Post(mRouter, "/api/bios/upload/*", Rest::Routes::bind(&IRouter::BiosUpload, this));
      // Systems
      Rest::Routes::Get(mRouter, "/api/systems", Rest::Routes::bind(&IRouter::SystemsGetAll, this));
      Rest::Routes::Get(mRouter, "/api/systems/getactives", Rest::Routes::bind(&IRouter::SystemsGetActives, this));
      Rest::Routes::Get(mRouter, "/api/systems/*/resource/*/svg/console", Rest::Routes::bind(&IRouter::SystemsResourceGetConsole, this));
      Rest::Routes::Get(mRouter, "/api/systems/*/resource/*/svg/controller", Rest::Routes::bind(&IRouter::SystemsResourceGetController, this));
      Rest::Routes::Get(mRouter, "/api/systems/*/resource/*/svg/controls", Rest::Routes::bind(&IRouter::SystemsResourceGetControls, this));
      Rest::Routes::Get(mRouter, "/api/systems/*/resource/*/svg/game", Rest::Routes::bind(&IRouter::SystemsResourceGetGame, this));
      Rest::Routes::Get(mRouter, "/api/systems/*/resource/*/svg/logo", Rest::Routes::bind(&IRouter::SystemsResourceGetLogo, this));
      // Configurations
      Rest::Routes::Get(mRouter, "/api/configuration/*", Rest::Routes::bind(&IRouter::ConfigurationGet, this));
      Rest::Routes::Post(mRouter, "/api/configuration/*", Rest::Routes::bind(&IRouter::ConfigurationSet, this));
      Rest::Routes::Options(mRouter, "/api/configuration/*", Rest::Routes::bind(&IRouter::ConfigurationOptions, this));
      Rest::Routes::Delete(mRouter, "/api/configuration/*", Rest::Routes::bind(&IRouter::ConfigurationDelete, this));
      Rest::Routes::Get(mRouter, "/api/configuration/system/*", Rest::Routes::bind(&IRouter::SystemConfigurationGet, this));
      Rest::Routes::Post(mRouter, "/api/configuration/system/*", Rest::Routes::bind(&IRouter::SystemConfigurationSet, this));
      Rest::Routes::Options(mRouter, "/api/configuration/system/*", Rest::Routes::bind(&IRouter::SystemConfigurationOptions, this));
      Rest::Routes::Delete(mRouter, "/api/configuration/system/*", Rest::Routes::bind(&IRouter::SystemConfigurationDelete, this));
      // Screenshots/Videos
      Rest::Routes::Options(mRouter, "/api/media/*", Rest::Routes::bind(&IRouter::MediaOptions, this));
      Rest::Routes::Get(mRouter, "/api/media", Rest::Routes::bind(&IRouter::MediaGetList, this));
      Rest::Routes::Delete(mRouter, "/api/media/*", Rest::Routes::bind(&IRouter::MediaDelete, this));
      Rest::Routes::Post(mRouter, "/api/media/takescreenshot", Rest::Routes::bind(&IRouter::MediaTakeScreenshot, this));
      Rest::Routes::Get(mRouter, "/api/media/*", Rest::Routes::bind(&IRouter::MediaGet, this));
      Rest::Routes::Get(mRouter, "/api/media/screenshot/*", Rest::Routes::bind(&IRouter::MediaGetScreenshot, this));

      // Default file service
      Rest::Routes::NotFound(mRouter, Rest::Routes::bind(&IRouter::FileServer, this));

      LOG(LogInfo) << "[RestAPIRouter] Router initialized";
    }

    //! Virtual destructor
    virtual ~IRouter() = default;

    /*!
     * @brief Get route handler
     * @return Route handler
     */
    std::shared_ptr<Rest::Private::RouterHandler> Handler() const { return mRouter.handler(); }
};
