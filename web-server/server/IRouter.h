//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include <pistache/router.h>
#include <pistache/http.h>
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
     * @brief Handle POST to reboot the system
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemReboot(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST to shutdown the system
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemShutdown(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST to start ES
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemEsStart(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST to stop ES
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemEsStop(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle POST to restart ES
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemEsRestart(const Rest::Request& request, Http::ResponseWriter response) = 0;

    /*!
     * @brief Handle GET to generate support archive
     * @param request Request object
     * @param response Response object
     */
    virtual void SystemSupportArchive(const Rest::Request& request, Http::ResponseWriter response) = 0;

  public:
    /*!
     * @brief Constructor. Set all routes
     */
    IRouter()
    {
      // System
      Rest::Routes::Post(mRouter, "/api/system/reboot", Rest::Routes::bind(&IRouter::SystemReboot, this));
      Rest::Routes::Post(mRouter, "/api/system/shutdown", Rest::Routes::bind(&IRouter::SystemShutdown, this));
      Rest::Routes::Get(mRouter, "/api/system/supportarchive", Rest::Routes::bind(&IRouter::SystemSupportArchive, this));

      // Frontend
      Rest::Routes::Post(mRouter, "/api/system/frontend/start", Rest::Routes::bind(&IRouter::SystemEsStart, this));
      Rest::Routes::Post(mRouter, "/api/system/frontend/stop", Rest::Routes::bind(&IRouter::SystemEsStop, this));
      Rest::Routes::Post(mRouter, "/api/system/frontend/restart", Rest::Routes::bind(&IRouter::SystemEsRestart, this));

      // Default file service
      Rest::Routes::NotFound(mRouter, Rest::Routes::bind(&IRouter::FileServer, this));

      LOG(LogInfo) << "[RestAPIRouter] Router initialized";
    }

    /*!
     * @brief Get route handler
     * @return Route handler
     */
    std::shared_ptr<Rest::Private::RouterHandler> Handler() const { return mRouter.handler(); }
};
