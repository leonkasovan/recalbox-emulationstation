/* 
 * File:   Upgrade.cpp
 * Author: matthieu
 * 
 * Created on 6 February 2015, 11:40
 */

#include <arpa/nameser.h>
#include <resolv.h>
#include "utils/network/HttpClient.h"
#include <utils/Files.h>
#include "Upgrade.h"
#include "RecalboxConf.h"
#include "utils/locale/LocaleHelper.h"
#include "recalbox/RecalboxSystem.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiMsgBoxScroll.h"
#include "guis/GuiUpdateRecalbox.h"
#include <patreon/PatronInfo.h>
#include <guis/GuiInfoPopup.h>

String Upgrade::mDomainName;
String Upgrade::mRemoteVersion;
String Upgrade::mLocalVersion;
String Upgrade::mRemoteReleaseNote;
String Upgrade::mLocalReleaseNote;

Upgrade::Upgrade(WindowManager& window)
  : mWindow(window)
  , mSender(*this)
{
  Thread::Start("Upgrade");
}

Upgrade::~Upgrade()
{
  mSignal.Fire();
  Thread::Stop();
}

void Upgrade::Run()
{
  try
  {
    // Wait for patron response
    PatronInfo::Instance().WaitForAuthentication(*this);

    // First check at 15s
    int waitForSeconds = 15;
    while (IsRunning())
    {
      if (mSignal.WaitSignal(waitForSeconds * 1000LL)) return;

      // Next checks, once an hour
      waitForSeconds = 3600;

      // Wait for network being available
      while(IsRunning())
      {
        if (mSignal.WaitSignal(5000LL)) return;
        if (RecalboxSystem::hasIpAdress(false)) break;
      }

      // Do we have to update?
      mRemoteVersion = GetRemoteVersion();
      mLocalVersion = Files::LoadFile(Path(sLocalVersionFile)).Trim();
      mRemoteReleaseNote = GetRemoteReleaseVersion();
      mLocalReleaseNote = Files::LoadFile(Path(sLocalReleaseNoteFile)).Trim();

      if (ValidateVersion(mRemoteVersion))
      {
        if (mRemoteVersion != mLocalVersion)
        {
          { LOG(LogInfo) << "[Update] Remote version " << mRemoteVersion << " does not match local version " << mLocalVersion << ". Update available!"; }

          // Popup, always shown
          mPopupMessage = _("AN UPDATE IS AVAILABLE FOR YOUR RECALBOX");
          mPopupMessage += "\n";
          mPopupMessage += mRemoteVersion;
          mPopupMessage += "\n\n";
          mPopupMessage += _("You're strongly recommended to update your Recalbox.\nNo support will be provided for older versions!");

          // Message box only if the option is on
          if (RecalboxConf::Instance().AsBool("updates.enabled"))
          {
            while (mWindow.HasGui() || mWindow.isSleeping())
              Thread::Sleep(5000);

            mMessageBoxMessage = _("NEW VERSION:");
            mMessageBoxMessage += " ";
            mMessageBoxMessage += mRemoteVersion;

            if (!mRemoteReleaseNote.empty())
            {
              mMessageBoxMessage += "\n\n";
              mMessageBoxMessage += _("CHANGELOG");
              mMessageBoxMessage += ":\n\n";
              mMessageBoxMessage += mRemoteReleaseNote;
            }
          }

          mSender.Send();
        }
        else { LOG(LogInfo) << "[Update] Remote version match local version. No update."; }
      }
      else { LOG(LogError) << "[Update] Invalid remote version! " << mRemoteVersion; }
    }
  }
  catch(std::exception& ex)
  {
    { LOG(LogError) << "[Update] Upgrade thread crashed."; }
    { LOG(LogError) << "[Update] Exception: " << ex.what(); }
  }
}

void Upgrade::ReceiveSyncMessage()
{
  // Volatile popup
  mWindow.InfoPopupAdd(new GuiInfoPopup(mWindow, mPopupMessage, 10, PopupType::Recalbox));

  // Messagebox
  if (!mMessageBoxMessage.empty())
  {
    //mWindow.displayScrollMessage(_("AN UPDATE IS AVAILABLE FOR YOUR RECALBOX"), mMessageBoxMessage, false);
    Gui* gui = new GuiMsgBoxScroll(mWindow, _("AN UPDATE IS AVAILABLE FOR YOUR RECALBOX"), mMessageBoxMessage, _("LATER"), nullptr, _("UPDATE NOW"),
                                   [this] { mWindow.pushGui(new GuiUpdateRecalbox(mWindow, TarUrl(), ImageUrl(), HashUrl(), NewVersion())); },
                                   String::Empty, nullptr, TextAlignment::Left);
    mWindow.pushGui(gui);
  }
}

String Upgrade::GetDomainName()
{
  if (!mDomainName.empty()) return mDomainName;

  // Select DNS to query
  String target = RecalboxConf::Instance().GetUpdatesType().LowerCase().Trim();
  // If target has been set to patron, we set it as not existing, to avoid the upgrade if the key is not valid
  if(target == "patron")
    target = "not-existing";
  // And if we are a patron, we can upgrade
  if (PatronInfo::Instance().IsPatron() && target != "alpha" && target != "jamma")
    target = "patron";
  target.Remove(' ');
  String domain(target);
  domain.Append(sUpgradeDNS);

  { LOG(LogDebug) << "[Update] " << (PatronInfo::Instance().IsPatron() ? "As a patron" : "") << " updates.type implied dns to use: " << domain; }

  // Query TXT
  unsigned char buffer[4096];
  int l = res_query(domain.c_str(), C_IN, ns_t_txt, buffer, sizeof (buffer));
  if (l >= 0)
  {
    ns_msg msg;
    ns_initparse(buffer, l, &msg);
    ns_rr rr;
    if (ns_parserr(&msg, ns_s_an, 0, &rr) == 0)
    {
      mDomainName = String((char*) (ns_rr_rdata(rr) + 1), (int)ns_rr_rdata(rr)[0]);
      { LOG(LogDebug) << "[Update] Domain: " << mDomainName; }
    }
  }

  return mDomainName;
}

String Upgrade::GetRemoteVersion()
{
  // Get version
  String url = ReplaceMachineParameters(sVersionPatternUrl, String::Empty);

  String version;
  HttpClient request;
  for(int i = 3; --i >= 0; )
    if (request.Execute(url, version))
    {
      int returnCode = request.GetLastHttpResponseCode();
      if (returnCode == 200) break;  // Exit for
      if (returnCode >= 500 && returnCode <= 599) { Thread::Sleep(5000); continue; } // Next loop
      { LOG(LogError) << "[Update] Error getting remote version: " << url << " - got: " << request.GetLastHttpResponseCode() ; }
      version.clear();
      break;
    }
    else
    {
      { LOG(LogError) << "[Update] Error getting remote version: " << url; }
      version.clear();
    }

  if (version.StartsWith('<')) version.clear();
  version.Trim(" \t\r\n");
  { LOG(LogDebug) << "[Update] Remote version: " << version << " (" << url << ')'; }

  // Return version
  return version;
}

String Upgrade::TarUrl()
{
  // Get url
  String url = ReplaceMachineParameters(sTarDownloadPatternUrl, String::Empty);
  { LOG(LogDebug) << "[Update] Tar file url: " << url; }

  return url;
}

String Upgrade::ImageUrl()
{
  // Get url
  String url = ReplaceMachineParameters(sDownloadPatternUrl, String::Empty);
  { LOG(LogDebug) << "[Update] Image file url: " << url; }

  return url;
}

String Upgrade::HashUrl()
{
  // Get url
  String url = ReplaceMachineParameters(sDownloadPatternUrl, ".sha1");
  { LOG(LogDebug) << "[Update] Hash file url: " << url; }

  return url;
}

String Upgrade::ReplaceMachineParameters(const String& url, const String& ext)
{
  String result(url);

  // Get domain
  GetDomainName();

  // Get arch
  String arch = Files::LoadFile(Path(sLocalArchFile));
  if (arch == "xu4") arch = "odroidxu4";

  // Get uuid
  String uuid = Files::LoadFile(Path(sLocalUUID)).Trim();
  if (uuid.empty()) uuid = "";

  // Replacements
  result.Replace("#DOMAIN#", mDomainName)
        .Replace("#ARCH#", arch)
        .Replace("#UUID#", uuid)
        .Replace("#EXT#", ext);

  return result;
}

bool Upgrade::ValidateVersion(const String& version)
{
  static String _allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._- ()/";
  if (!version.empty())
    if (version[0] >= '0' && version[0] <= '9')
      return (version.find_first_not_of(_allowedCharacters) == String::npos);

  return false;
}

String Upgrade::GetRemoteReleaseVersion()
{
  // Get version
  String url = ReplaceMachineParameters(sReleasenotePatternUrl, String::Empty);

  String releaseNote;
  HttpClient request;
  for(int i = 3; --i >= 0; )
    if (request.Execute(url, releaseNote))
    {
      int returnCode = request.GetLastHttpResponseCode();
      if (returnCode == 200) break;  // Exit for
      if (returnCode >= 500 && returnCode <= 599) { Thread::Sleep(5000); continue; } // Next loop
      { LOG(LogError) << "[Update] Error getting remote release note: " << url << " - got: " << request.GetLastHttpResponseCode() ; }
      releaseNote.clear();
      break;
    }
    else
    {
      { LOG(LogError) << "[Update] Error getting remote release note: " << url; }
      releaseNote.clear();
    }

  { LOG(LogDebug) << "[Update] Remote release note: " << releaseNote << " (" << url << ')'; }

  if (releaseNote.StartsWith('<')) releaseNote.clear();
  else releaseNote.Trim();

  // Return version
  return releaseNote;
}
