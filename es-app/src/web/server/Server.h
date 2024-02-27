//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include "IRouter.h"
#include <web/Parameters.h>
#include <pistache/endpoint.h>
#include <pistache/net.h>

class Server
{
  private:
    //! Address to listen on
    Pistache::Address mAddress;
    //! Http server
    Http::Endpoint mServer;

  public:
    /*!
     * @brief Default Constructor
     */
    Server(const Parameters& param, IRouter* router);

    /*!
     * @brief Run the server
     */
    void Serve();

    /*!
     * @brief Stop the server
     */
    void Shutdown();
};
