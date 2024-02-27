//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include "../IRouter.h"

class RequestHandler : public IRouter
{
  private:
    //! WWW root
    Path mWWWRoot;
    //! WWW default file
    std::string mDefaultFile;

  public:
    /*!
     * @brief Constructor
     * @param wwwRoot Root path to files to serve
     * @param defaultFile Default file to serve
     */
    RequestHandler(const std::string& wwwRoot, const std::string& defaultFile)
      : mWWWRoot(wwwRoot),
        mDefaultFile(defaultFile)
    {
    }

    //! Default virtual destructor
    virtual ~RequestHandler() = default;

    /*!
     * @brief Handle files
     * @param request Request object
     * @param response Response object
     */
    void FileServer(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle POST to reboot the system
     * @param request Request object
     * @param response Response object
     */
    void SystemReboot(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle POST to shutdown the system
     * @param request Request object
     * @param response Response object
     */
    void SystemShutdown(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to start ES
     * @param request Request object
     * @param response Response object
     */
    void SystemEsStart(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to stop ES
     * @param request Request object
     * @param response Response object
     */
    void SystemEsStop(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to restart ES
     * @param request Request object
     * @param response Response object
     */
    void SystemEsRestart(const Rest::Request& request, Http::ResponseWriter response) override;

    /*!
     * @brief Handle GET to generate support archive
     * @param request  Request object
     * @param response Response object
     */
    void SystemSupportArchive(const Rest::Request& request, Http::ResponseWriter response) override;
};
