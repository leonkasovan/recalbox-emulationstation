//
// Created by bkg2k on 30/03/2020.
//

#include <csignal>
#include "Parameters.h"
#include "server/handlers/RequestHandler.h"
#include "server/Server.h"
#include <utils/Log.h>

static Server* serverInstance = nullptr;

void my_handler(int s)
{
  LOG(LogError) << "Signal " << s << " received.";
  switch(s)
  {
    case SIGTERM:
    case SIGQUIT:
    case SIGKILL:
    {
      if (serverInstance != nullptr)
        serverInstance->Cancel();
      // Let the server shutdown gracefully
      usleep(1000000);
      break;
    }
    default:
    {
      abort();
    }
  }
  exit(0);
}

void InstallSignalCatcher()
{
  struct sigaction sigIntHandler{};

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);
  sigaction(SIGILL, &sigIntHandler, nullptr);
  sigaction(SIGABRT, &sigIntHandler, nullptr);
  sigaction(SIGFPE, &sigIntHandler, nullptr);
  sigaction(SIGSEGV, &sigIntHandler, nullptr);
  sigaction(SIGTERM, &sigIntHandler, nullptr);
  sigaction(SIGQUIT, &sigIntHandler, nullptr);
  sigaction(SIGKILL, &sigIntHandler, nullptr);
  sigaction(SIGBUS, &sigIntHandler, nullptr);
  sigaction(SIGSYS, &sigIntHandler, nullptr);
}

int main(int argc, char* argv[])
{
  InstallSignalCatcher();

  // Get parameters
  Parameters param(argc, argv);

  ::Log::Open("web-server.log");
  if (param.Debug()) ::Log::SetReportingLevel(LogLevel::LogDebug);

  LOG(LogInfo) << "Recalbox Micro Webserver 1.0";

  param.LogConfig();

  // Build server
  RequestHandler requestHandler(param.WWWRoot(), param.DefaultFile());
  Server server(param, &requestHandler);

  // Run!
  serverInstance = &server;
  server.Run();
  serverInstance = nullptr;
}