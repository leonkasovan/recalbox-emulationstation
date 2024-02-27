//
// Created by bkg2k on 13/07/23.
//

#include "RestApiServer.h"
#include <csignal>
#include "Parameters.h"
#include "server/handlers/RequestHandler.h"
#include "server/Server.h"
#include <utils/Log.h>

RestApiServer::RestApiServer(SystemManager& systemManager)
  : mRequestHandler(mParam.WWWRoot(), mParam.DefaultFile(), systemManager)
  , mServer(mParam, &mRequestHandler)
{
  Start("rest-api");
}

void RestApiServer::Run()
{
  LOG(LogInfo) << "[RestAPIServer] Recalbox WebApi Server 1.0";

  // Run!
  while(IsRunning())
    try
    {
      mServer.Serve();
      break;
    }
    catch(std::exception& ex)
    {
      LOG(LogError) << "[RestAPIServer] Error running server! Retrying in 5s... (Exception: " << ex.what() << ')';
      Thread::Sleep(5000);
    }

  mServer.Shutdown();
}

void RestApiServer::Break()
{
  mServer.Shutdown();
}
