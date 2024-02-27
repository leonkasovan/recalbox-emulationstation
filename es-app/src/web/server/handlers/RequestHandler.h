//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include <web/server/IRouter.h>
#include <web/server/handlers/providers/SysInfos.h>
#include <bios/BiosManager.h>
#include <mqtt/MqttClient.h>
#include <web/server/handlers/providers/EmulationStationWatcher.h>
#include "systems/SystemManager.h"

class RequestHandler : public IRouter
{
  private:
    //! Bios Manager
    BiosManager& mBiosManager;
    //! SystemManager reference
    SystemManager& mSystemManager;
    //! System information provider
    SysInfos mSysInfos;
    //! Event watcher
    EmulationStationWatcher mWatcher;

    //! WWW root
    Path mWWWRoot;
    //! WWW default file
    String mDefaultFile;

    /*!
     * @brief Decode the given string using base64 algorythm
     * @param base64 String to decode
     * @return Decoded string
     */
    static String Decode64(const String& base64);

  public:
    RequestHandler(const String& wwwRoot, const String& defaultFile, SystemManager& systemManager)
      : mBiosManager(BiosManager::Instance())
      , mSystemManager(systemManager)
      , mWWWRoot(wwwRoot)
      , mDefaultFile(defaultFile)
    {
    }

    //! default virtual destructor
    ~RequestHandler() override = default;

    /*!
     * @brief Handle files
     * @param request Request object
     * @param response Response object
     */
    void FileServer(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    void Versions(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET Architecture
     * @param request Request object
     * @param response Response object
     */
    void Architecture(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    void SystemInfo(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    void StorageInfo(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    void BiosDownload(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET version
     * @param request Request object
     * @param response Response object
     */
    void BiosUpload(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get array of all system's bios
     * @param request Request object
     * @param response Response object
     */
    void BiosGetAll(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get bios for the given system
     * @param request Request object
     * @param response Response object
     */
    void BiosGetSystem(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get all systems
     * @param request Request object
     * @param response Response object
     */
    void SystemsGetAll(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get all active  systems
     * @param request Request object
     * @param response Response object
     */
    void SystemsGetActives(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system console svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    void SystemsResourceGetConsole(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system controller svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    void SystemsResourceGetController(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system controls svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    void SystemsResourceGetControls(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system game support svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    void SystemsResourceGetGame(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system logo svg for the given system and region
     * @param request Request object
     * @param response Response object
     */
    void SystemsResourceGetLogo(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get configuration keys
     * @param request Request object
     * @param response Response object
     */
    void ConfigurationGet(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle POST set configuration keys
     * @param request Request object
     * @param response Response object
     */
    void ConfigurationSet(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle OPTIONS get configuration keys, type and validators
     * @param request Request object
     * @param response Response object
     */
    void ConfigurationOptions(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle DELETE delete configuration keys
     * @param request Request object
     * @param response Response object
     */
    void ConfigurationDelete(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get system configuration keys
     * @param request Request object
     * @param response Response object
     */
    void SystemConfigurationGet(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle POST set system configuration keys
     * @param request Request object
     * @param response Response object
     */
    void SystemConfigurationSet(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle OPTIONS get system configuration keys, type and validators
     * @param request Request object
     * @param response Response object
     */
    void SystemConfigurationOptions(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle DELETE delete system configuration keys
     * @param request Request object
     * @param response Response object
     */
    void SystemConfigurationDelete(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle OPTIONS get media options
     * @param request Request object
     * @param response Response object
     */
    void MediaOptions(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET get media list
     * @param request Request object
     * @param response Response object
     */
    void MediaGetList(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle DELETE delete media
     * @param request Request object
     * @param response Response object
     */
    void MediaDelete(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle POST create screenshot
     * @param request Request object
     * @param response Response object
     */
    void MediaTakeScreenshot(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to get game media
     * @param request Request object
     * @param response Response object
     */
    void MediaGet(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to get screenshot media
     * @param request Request object
     * @param response Response object
     */
    void MediaGetScreenshot(const Rest::Request& request, Http::ResponseWriter response) override;
};
