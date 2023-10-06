//
// Created by Bkg2k on 10/03/2020.
//

#include <utils/storage/HashMap.h>
#include <utils/Files.h>
#include <systems/SystemData.h>
#include <systems/SystemManager.h>
#include <VideoEngine.h>
#include <sys/wait.h>
#include "NotificationManager.h"
#include <spawn.h>

/*
 * Members
 */
const String eol("\r\n");
const Path NotificationManager::sStatusFilePath("/tmp/es_state.inf");
HashMap<String, pid_t> NotificationManager::sPermanentScriptsPID;

NotificationManager::NotificationManager(char** environment)
  : StaticLifeCycleControler<NotificationManager>("NotificationManager"),
    mMQTTClient("recalbox-emulationstation", nullptr),
    mEnvironment(environment)
{
  LoadScriptList();
}

const char* NotificationManager::ActionToString(Notification action)
{
  switch(action)
  {
    case Notification::None:                 return "none";
    case Notification::Start:                return "start";
    case Notification::Stop:                 return "stop";
    case Notification::Shutdown:             return "shutdown";
    case Notification::Reboot:               return "reboot";
    case Notification::Relaunch:             return "relaunch";
    case Notification::Quit:                 return "quit";
    case Notification::SystemBrowsing:       return "systembrowsing";
    case Notification::GamelistBrowsing:     return "gamelistbrowsing";
    case Notification::RunKodi:              return "runkodi";
    case Notification::RunGame:              return "rungame";
    case Notification::RunDemo:              return "rundemo";
    case Notification::EndGame:              return "endgame";
    case Notification::EndDemo:              return "enddemo";
    case Notification::Sleep:                return "sleep";
    case Notification::WakeUp:               return "wakeup";
    case Notification::ScrapStart:           return "scrapstart";
    case Notification::ScrapStop:            return "scrapstop";
    case Notification::ScrapGame:            return "scrapgame";
    case Notification::ConfigurationChanged: return "configurationchanged";
    case Notification::StartGameClip:        return "startgameclip";
    case Notification::StopGameClip:         return "stopgameclip";
    default: break;
  }
  return "error";
}

Notification NotificationManager::ActionFromString(const String& action)
{
  static HashMap<String, Notification> sStringToAction
  ({
    { "start"               , Notification::Start                },
    { "stop"                , Notification::Stop                 },
    { "shutdown"            , Notification::Shutdown             },
    { "reboot"              , Notification::Reboot               },
    { "relaunch"            , Notification::Relaunch             },
    { "quit"                , Notification::Quit                 },
    { "systembrowsing"      , Notification::SystemBrowsing       },
    { "gamelistbrowsing"    , Notification::GamelistBrowsing     },
    { "runkodi"             , Notification::RunKodi              },
    { "rungame"             , Notification::RunGame              },
    { "rundemo"             , Notification::RunDemo              },
    { "endgame"             , Notification::EndGame              },
    { "enddemo"             , Notification::EndDemo              },
    { "sleep"               , Notification::Sleep                },
    { "wakeup"              , Notification::WakeUp               },
    { "scrapstart"          , Notification::ScrapStart           },
    { "scrapstop"           , Notification::ScrapStop            },
    { "scrapgame"           , Notification::ScrapGame            },
    { "configurationchanged", Notification::ConfigurationChanged },
    { "startgameclip"       , Notification::StartGameClip        },
    { "stopgameclip"        , Notification::StopGameClip         },
  });

  if (!sStringToAction.contains(action))
    return Notification::None;

  return sStringToAction[action];
}

bool NotificationManager::ExtractSyncFlagFromPath(const Path& path)
{
  const String& scriptName = path.FilenameWithoutExtension().LowerCase();
  return (scriptName.find("(sync)") != String::npos);
}

bool NotificationManager::ExtractPermanentFlagFromPath(const Path& path)
{
  const String& scriptName = path.FilenameWithoutExtension().LowerCase();
  return (scriptName.find("(permanent)") != String::npos);
}

Notification NotificationManager::ExtractNotificationsFromPath(const Path& path)
{
  // Extract events between [ and ] in filename
  const String& scriptName = path.FilenameWithoutExtension().LowerCase();
  unsigned long start = scriptName.find('[');
  unsigned long stop = scriptName.find(']');

  if (((start | stop) == String::npos) || (stop - start <= 1)) return (Notification)-1;

  Notification result = Notification::None;
  // Split events
  String::List events = scriptName.SubString(start + 1, stop - start - 1).Split(',');
  // Extract notifications
  for(const String& event : events)
    result |= ActionFromString(event);
  return result;
}

void NotificationManager::LoadScriptList()
{
  Path scriptsFolder(sScriptPath);
  Path::PathList scripts = scriptsFolder.GetDirectoryContent();

  for(const Path& path : scripts)
    if (path.IsFile())
      if (HasValidExtension(path))
      {
        bool permanent = ExtractPermanentFlagFromPath(path);
        bool synced = ExtractSyncFlagFromPath(path) && !permanent;
        if (permanent)
        {
          RunProcess(path, {}, false, true);
          { LOG(LogInfo) << "[Script] Run permanent UserScript: " << path.ToString(); }
        }
        else
        {
          mScriptList.push_back({ path, ExtractNotificationsFromPath(path), synced });
          { LOG(LogInfo) << "[Script] Scan UserScript: " << path.ToString(); }
        }
      }
}

NotificationManager::RefScriptList NotificationManager::FilteredScriptList(Notification action)
{
  RefScriptList result;

  for(const ScriptData& script : mScriptList)
    if ((script.mFilter & action) != 0)
      result.push_back(&script);

  return result;
}

void NotificationManager::RunScripts(Notification action, const String& param)
{
  RefScriptList scripts = FilteredScriptList(action);
  if (scripts.empty()) return; // Nothing to launch

  // Build parameter
  String::List args;
  args.push_back("-action");
  args.push_back(ActionToString(action));
  args.push_back("-statefile");
  args.push_back(sStatusFilePath.ToString());
  if (!param.empty())
  {
    args.push_back("-param");
    args.push_back(param);
  }

  for(const ScriptData* script : scripts)
  {
    // Run!
    RunProcess(script->mPath, args, script->mSync, false);
  }
}

void NotificationManager::BuildStateCommons(String& output, const SystemData* system, const FileData* game, Notification action, const String& actionParameters)
{
  // Build status file
  output.Append("Action=").Append(ActionToString(action)).Append(eol)
        .Append("ActionData=").Append(actionParameters).Append(eol);

  // System
  if (system != nullptr)
    output.Append("System=").Append(system->FullName()).Append(eol)
          .Append("SystemId=").Append(system->Name()).Append(eol);
  else if (action == Notification::RunKodi)
    output.Append("System=kodi").Append(eol)
          .Append("SystemId=kodi").Append(eol);
  else
    output.Append("System=").Append(eol)
          .Append("SystemId=").Append(eol);

  // Permanent game infos
  if (game != nullptr)
    output.Append("Game=").Append(game->Name()).Append(eol)
          .Append("GamePath=").Append(game->RomPath().ToString()).Append(eol)
          .Append("ImagePath=").Append(game->Metadata().Image().ToString()).Append(eol);
  else
    output.Append("Game=").Append(eol)
          .Append("GamePath=").Append(eol)
          .Append("ImagePath=").Append(eol);
}

void NotificationManager::BuildStateGame(String& output, const FileData* game, Notification action)
{
  if (game == nullptr) return;

  output.Append("IsFolder=").Append(game->IsFolder() ? "1" : "0").Append(eol)
        .Append("ThumbnailPath=").Append(game->Metadata().Thumbnail().ToString()).Append(eol)
        .Append("VideoPath=").Append(game->Metadata().Video().ToString()).Append(eol)
        .Append("Developer=").Append(game->Metadata().Developer()).Append(eol)
        .Append("Publisher=").Append(game->Metadata().Publisher()).Append(eol)
        .Append("Players=").Append(game->Metadata().PlayersAsString()).Append(eol)
        .Append("Region=").Append(game->Metadata().RegionAsString()).Append(eol)
        .Append("Genre=").Append(game->Metadata().Genre()).Append(eol)
        .Append("GenreId=").Append(game->Metadata().GenreIdAsString()).Append(eol)
        .Append("Favorite=").Append(game->Metadata().Favorite() ? "1" : "0").Append(eol)
        .Append("Hidden=").Append(game->Metadata().Hidden() ? "1" : "0").Append(eol)
        .Append("Adult=").Append(game->Metadata().Adult() ? "1" : "0").Append(eol);

  if (action != Notification::ScrapGame)
  {
    String emulator;
    String core;
    if (game->System().Manager().Emulators().GetGameEmulator(*game, emulator, core))
      output.Append("Emulator=").Append(emulator).Append(eol).Append("Core=").Append(core).Append(eol);
  }
}

void NotificationManager::BuildStateSystem(String& output, const SystemData* system, Notification action)
{
  if (system == nullptr) return;

  if (!system->IsVirtual() && action != Notification::ScrapGame)
  {
    String emulator;
    String core;
    if (system->Manager().Emulators().GetDefaultEmulator(*system, emulator, core))
      output.Append("DefaultEmulator=").Append(emulator).Append(eol).Append("DefaultCore=").Append(core).Append(eol);
  }
}

void NotificationManager::BuildStateCompatibility(String& output, Notification action)
{
  // Mimic old behavior of "State"
  output.Append("State=");
  switch(action)
  {
    case Notification::RunKodi:
    case Notification::RunGame: output.Append("playing"); break;
    case Notification::RunDemo: output.Append("demo"); break;
    case Notification::None:
    case Notification::Start:
    case Notification::Stop:
    case Notification::Shutdown:
    case Notification::Reboot:
    case Notification::Relaunch:
    case Notification::Quit:
    case Notification::SystemBrowsing:
    case Notification::GamelistBrowsing:
    case Notification::EndGame:
    case Notification::EndDemo:
    case Notification::Sleep:
    case Notification::WakeUp:
    case Notification::ScrapStart:
    case Notification::ScrapStop:
    case Notification::ScrapGame:
    case Notification::ConfigurationChanged:
    case Notification::StartGameClip:
    case Notification::StopGameClip:
    default: output.Append("selected"); break;
  }
}

void NotificationManager::Notify(const SystemData* system, const FileData* game, Notification action, const String& actionParameters)
{
//  why ?
//  if (VideoEngine::IsInstantiated())
//    VideoEngine::Instance().StopVideo();

  String notificationParameter;
  
  if (game != nullptr){
    notificationParameter = game->RomPath().ToString();
  }else{
    if (system != nullptr){
      notificationParameter = system->Name();
    }else{
      notificationParameter = actionParameters;
    }
  }
  // Check if it is the same event than in previous call
  ParamBag newBag(system, game, action, notificationParameter);
  if (newBag != mPreviousParamBag)
  {
    // Build all
    String output("Version=2.0"); output.Append(eol);
    BuildStateCommons(output, system, game, action, notificationParameter);
    BuildStateGame(output, game, action);
    BuildStateSystem(output, system, action);
    BuildStateCompatibility(output, action);

    // Save
    Files::SaveFile(Path(sStatusFilePath), output);

    // MQTT notification
    mMQTTClient.Send(sEventTopic, ActionToString(action));

    // Run scripts
    RunScripts(action, notificationParameter);
  }
  mPreviousParamBag = newBag;
}

void NotificationManager::RunProcess(const Path& target, const String::List& arguments, bool synchronous, bool permanent)
{
  // final argument array
  std::vector<const char*> args;

  String command;

  // Extract extension
  String ext = target.Extension().LowerCase();
  if      (ext == ".sh")  { command = "/bin/sh";          args.push_back(command.data()); }
  else if (ext == ".ash") { command = "/bin/ash";         args.push_back(command.data()); }
  else if (ext == ".lua") { command = "/usr/bin/lua";     args.push_back(command.data()); }
  else if (ext == ".luax"){ command = "/usr/bin/luax";    args.push_back(command.data()); }
  else if (ext == ".py")  { command = "/usr/bin/python";  args.push_back(command.data()); }
  else if (ext == ".py2") { command = "/usr/bin/python2"; args.push_back(command.data()); }
  else if (ext == ".py3") { command = "/usr/bin/python3"; args.push_back(command.data()); }
  else { command = target.ToString(); }

  args.push_back(target.ToChars());
  for (const String& argument : arguments) args.push_back(argument.c_str());

  { LOG(LogInfo) << "[Script][novan] Run UserScript: " << args; }

  // Push final null
  args.push_back(nullptr);

  if (sPermanentScriptsPID.contains(target.ToString()))
  {
    // Still running?
    if (waitpid(sPermanentScriptsPID[target.ToString()], nullptr, WNOHANG) == 0)
      return;
    // Not running, remove pid
    sPermanentScriptsPID.erase(target.ToString());
  }

  pid_t pid = 0;
  posix_spawnattr_t spawn_attr;
  posix_spawnattr_init(&spawn_attr);
  int status = posix_spawn(&pid, command.data(), nullptr, &spawn_attr, (char **) args.data(), mEnvironment);
  posix_spawnattr_destroy(&spawn_attr);

  if (status != 0) // Error
  {
    { LOG(LogError) << "[Script] Error running " << target.ToString() << " (spawn failed)"; }
    return;
  }

  // Wait for child?
  if (synchronous)
  {
    if (waitpid(pid, &status, 0) != pid)
    { LOG(LogError) << "[Script] Error waiting for " << target.ToString() << " to complete. (waitpid failed)"; }
  }

  // Permanent?
  if (permanent)
    sPermanentScriptsPID.insert(target.ToString(), pid);
}

bool NotificationManager::HasValidExtension(const Path& path)
{
  String ext = path.Extension().LowerCase();
  return (ext.empty()) ||
         (ext == ".sh" ) ||
         (ext == ".ash") ||
         (ext == ".lua" ) ||
         (ext == ".luax" ) ||
         (ext == ".py" ) ||
         (ext == ".py2") ||
         (ext == ".py3");
}


