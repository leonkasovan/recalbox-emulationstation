//
// Created by bkg2k on 30/03/2020.
//

#include <cassert>
#include "Server.h"
#include "utils/os/system/Thread.h"

Server::Server(const Parameters& param, IRouter* router)
  : mAddress(param.IP(), Pistache::Port(param.Port()))
  , mServer(mAddress)
{
  assert(router != nullptr);

  Http::Endpoint::Options options = Http::Endpoint::options().threads(param.Threads());
  mServer.init(options);
  mServer.setHandler(router->Handler());
}

void Server::Serve()
{
  LOG(LogInfo) << "[WebServer] Launching Server!";
  mServer.serve();
}

void Server::Shutdown()
{
  LOG(LogInfo) << "[WebServer] Server interrupted!";
  mServer.shutdown();
}
