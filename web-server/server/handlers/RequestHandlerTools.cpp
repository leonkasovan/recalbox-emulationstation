//
// Created by bkg2k on 03/04/2020.
// Last modification by Maksthorr on 28/04/2023
//

#include <pistache/include/pistache/router.h>
#include <utils/Log.h>
#include "RequestHandlerTools.h"
#include "Mime.h"

using namespace Pistache;

String RequestHandlerTools::OutputOf(const String& command)
{
  String output;
  FILE* pipe = popen(command.c_str(), "r");
  if (pipe != nullptr)
  {
    char buffer[1024];
    while (feof(pipe) == 0)
      if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output.Append(buffer);
    pclose(pipe);
  }
  return output;
}

String::List RequestHandlerTools::OutputLinesOf(const String& command)
{
  return OutputOf(command).Split('\n');
}

void RequestHandlerTools::Error404(Http::ResponseWriter& response)
{
  LOG(LogError) << "Error 404";
  SetHeaders(response);
  Send(response, Http::Code::Not_Found, "404 - Not found", Mime::PlainText);
}

void RequestHandlerTools::LogRoute(const Pistache::Rest::Request& request, const char* methodName)
{
  LOG(LogDebug) << methodName << ": " << MethodToString(request.method()) << ' ' << request.resource();
}

const char* RequestHandlerTools::MethodToString(Pistache::Http::Method method)
{
  switch(method)
  {
    case Http::Method::Options: return "OPTIONS";
    case Http::Method::Get: return "GET";
    case Http::Method::Post: return "POST";
    case Http::Method::Head: return "HEAD";
    case Http::Method::Put: return "PUT";
    case Http::Method::Patch: return "PATCH";
    case Http::Method::Delete: return "DELETE";
    case Http::Method::Trace: return "TRACE";
    case Http::Method::Connect: return "CONNECT";

    // Avoid enum Warning
    case Http::Method::Acl:
    case Http::Method::BaselineControl:
    case Http::Method::Bind:
    case Http::Method::Checkin:
    case Http::Method::Checkout:
    case Http::Method::Copy:
    case Http::Method::Label:
    case Http::Method::Link:
    case Http::Method::Lock:
    case Http::Method::Merge:
    case Http::Method::Mkactivity:
    case Http::Method::Mkcalendar:
    case Http::Method::Mkcol:
    case Http::Method::Mkredirectref:
    case Http::Method::Mkworkspace:
    case Http::Method::Move:
    case Http::Method::Orderpatch:
    case Http::Method::Pri:
    case Http::Method::Propfind:
    case Http::Method::Proppatch:
    case Http::Method::Rebind:
    case Http::Method::Report:
    case Http::Method::Search:
    case Http::Method::Unbind:
    case Http::Method::Uncheckout:
    case Http::Method::Unlink:
    case Http::Method::Unlock:
    case Http::Method::Update:
    case Http::Method::Updateredirectref:
    case Http::Method::VersionControl:
    default: break;
  }
  return "UNKNOWN";
}

/*
 * Send
 */


void RequestHandlerTools::SetHeaders(Http::ResponseWriter& response)
{
  response.headers().add<Pistache::Http::Header::AccessControlAllowMethods>("GET, POST, OPTIONS, DELETE");
  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.headers().add<Pistache::Http::Header::AccessControlAllowHeaders>("*");
}

void
RequestHandlerTools::Send(Pistache::Http::ResponseWriter& response, Pistache::Http::Code code, const String& body,
                          const Pistache::Http::Mime::MediaType& mime)
{
  SetHeaders(response);
  response.send(code, body, mime);
}

Pistache::Async::Promise<ssize_t> RequestHandlerTools::Send(Pistache::Http::ResponseWriter& response, Pistache::Http::Code code)
{
  SetHeaders(response);
  return response.send(code, nullptr, 0, Pistache::Http::Mime::MediaType());
}

void RequestHandlerTools::SendResource(const Path& resourcepath, Http::ResponseWriter& response, const Pistache::Http::Mime::MediaType& mimeType)
{
  if (resourcepath.Exists())
  {
    SetHeaders(response);
    Http::serveFile(response, resourcepath.ToString(), mimeType);
  }
  else
    Error404(response);
}

void RequestHandlerTools::SendResource(const Path& preferedPath, const Path& fallbackPath, Http::ResponseWriter& response, const Http::Mime::MediaType& mimetype)
{
  if (preferedPath.Exists())
  {
    SetHeaders(response);
    Http::serveFile(response, preferedPath.ToString(), mimetype);
  }
  else if (fallbackPath.Exists())
  {
    SetHeaders(response);
    Http::serveFile(response, fallbackPath.ToString(), mimetype);
  }
  else
    Error404(response);
}

String RequestHandlerTools::GetCommandOutput(const String& command)
{
  String output;
  char buffer[10240];
  FILE* pipe = popen(command.c_str(), "r");
  if (pipe != nullptr)
  {
    while (feof(pipe) == 0)
      if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output.Append(buffer);
    pclose(pipe);
  }
  return output;
}
